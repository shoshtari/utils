#include "socket_manager.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

#include "socket.h"

typedef struct ackmap_key {
  unsigned short int seqnumber;
  struct sockaddr_in addr;
} ackmap_key;

int ackmap_key_compare(void *k1pointer, void *k2pointer) {
  return 0;
  ackmap_key k1 = *((ackmap_key *)k1pointer), k2 = *((ackmap_key *)k2pointer);

  if ((k1.addr.sin_addr.s_addr == k2.addr.sin_addr.s_addr) &&
      (k1.addr.sin_port == k2.addr.sin_port) &&
      (k1.seqnumber == k2.seqnumber)) {
    return 0;
  }
  return 1;
}

uint32_t ackmap_hash(void *rawBuffer, int cardinality) {
  ackmap_key *buffer = rawBuffer;
  uint32_t h = (buffer->addr.sin_addr.s_addr + buffer->seqnumber) % cardinality;
  /*printf("CALC_HASH %d %d %d\n", buffer->addr.sin_addr.s_addr,
   * buffer->seqnumber, h);*/
  return h;
}

socket_manager *new_socket_manager(int socket_fd) {
  socket_manager *manager = malloc(sizeof(socket_manager));
  manager->ack_map =
      create_hashsmap(ACKMAP_CARDINALITY, sizeof(ackmap_key), sizeof(int),
                      ackmap_key_compare, ackmap_hash);
  manager->recieve_buffer = create_linklist(sizeof(Packet));
  manager->recieve_notify = malloc(sizeof(sem_t));
  sem_init(manager->recieve_notify, 1, 0);

  manager->seqlock = malloc(sizeof(pthread_mutex_t));

  pthread_mutex_init(manager->seqlock, NULL);
  manager->seqnumber = 1;
  manager->fd = socket_fd;
  manager->running = 1;
  return manager;
}

unsigned short set_protocol_headers(socket_manager *manager, void *buf,
                                    unsigned short seqnumber) {
  // if seqnumber is not zero, then it is an ack package, else it generate and
  // set the seqnumber
  unsigned char ack = 0;
  if (seqnumber != 0) {
    ack = 1;
  }

  if (seqnumber == 0) {
    pthread_mutex_lock(manager->seqlock);
    seqnumber = manager->seqnumber;
    manager->seqnumber++;
    pthread_mutex_unlock(manager->seqlock);
  }
  unsigned char *acksection = buf;
  *acksection = ack;

  unsigned short *seqsection = buf + 1;
  *seqsection = seqnumber;

  return seqnumber;
}

Packet newPacket(struct sockaddr_in *addr, char *buf, int bufferSize) {
  Packet packet;

  packet.buffer = malloc(bufferSize);
  memcpy(packet.buffer, buf, bufferSize);

  packet.addr = malloc(sizeof(struct sockaddr_in));
  memcpy(packet.addr, addr, sizeof(struct sockaddr_in));

  packet.size = bufferSize;
  return packet;
}

void destroy_packet(Packet packet) {
  free(packet.buffer);
  free(packet.addr);
}

int send_and_wait_for_ack(socket_manager *manager, Packet packet,
                          unsigned short seqnumber) {
  ackmap_key *key = calloc(sizeof(ackmap_key), 1);
  key->addr = *packet.addr;
  key->seqnumber = seqnumber;

  int ackReceived = 0;
  hashmap_set(manager->ack_map, key, &ackReceived);
  void *check = hashmap_get(manager->ack_map, key);
  if (check == NULL) {
    printf("hashmap sucks!\n");
  }

  int count = 0;
  while (!ackReceived) {
    if (count > 0) {
      printf("send timeout reached for seq %d,hash: %d, retry count: %d \n",
             seqnumber, ackmap_hash(key, ACKMAP_CARDINALITY), count);
    }
    if (count >= MAX_RETRY) {
      printf("couldn't send packet");
      exit(1);
      break;
    }
    if (send_to_socket(manager->fd, packet.buffer, packet.size, packet.addr) <
        0) {
      printf("couldn't send data to socket");
      exit(1);
      break;
    }

    count++;
	void* mapValue = hashmap_get(manager->ack_map, key);
	if (mapValue == NULL){
		printf("hashamp sucks!\n");
		return -1;
	}
    ackReceived = *((int *)mapValue);
    usleep(RETRY_SLEEP); // 100ms
  }

  hashmap_del(manager->ack_map, key);
  free(key);
  return 0;
}

int send_ack(socket_manager *manager, char *received_buf,
             struct sockaddr_in *addr) {

  unsigned short seqnumber = *((unsigned short *)(received_buf + 1));
  char *buf = malloc(PROTOCOL_OVERHEAD);
  set_protocol_headers(manager, buf, seqnumber);
  printf("sending ack %hu\n", seqnumber);

  int ans = send_to_socket(manager->fd, buf, PROTOCOL_OVERHEAD, addr);
  free(buf);
  return ans;
}

int handle_recieved_app_packet(socket_manager *manager, Packet packet) {
  if (send_ack(manager, packet.buffer, packet.addr) < 0) {
    printf("couldn't send ack");
    return -1;
  }

  rinsert_linklist(manager->recieve_buffer, &packet);
  // printf("WRITE2 %d %p %p\n", manager->recieve_buffer->size,
  // manager->recieve_buffer->root, manager->recieve_buffer->end);

  sem_post(manager->recieve_notify);
  return 0;
}
int handle_recieved_ack_packet(socket_manager *manager, Packet packet) {

  ackmap_key *key = calloc(sizeof(ackmap_key), 1);
  key->addr = *packet.addr;
  key->seqnumber = *((unsigned short *)(packet.buffer + 1));

  destroy_packet(packet);
  /*printf("SEQ %hu acked successfully, hash: %d size: %d\n", key->seqnumber,*/
  /*       ackmap_hash(key, ACKMAP_CARDINALITY), manager->ack_map->size);*/
  int *status = hashmap_get(manager->ack_map, key);
  free(key);

  if (status == NULL) {
    return 0;
  }

  *status = 1;
  return 0;
}

unsigned short app_send(socket_manager *manager, Packet packet) {
  if (packet.size + PROTOCOL_OVERHEAD > DG_MAXSIZE) {
    printf("packet too big");
    return 0;
  }

  char *new_buffer = malloc(packet.size + PROTOCOL_OVERHEAD);
  unsigned short seqnumber = set_protocol_headers(manager, new_buffer, 0);
  memcpy(new_buffer + PROTOCOL_OVERHEAD, packet.buffer, packet.size);
  packet.buffer = new_buffer;
  packet.size += PROTOCOL_OVERHEAD;

  if (packet.size > 0) {
    printf("sending: %u %hu to %d:%d \n", packet.buffer[0],
           *(packet.buffer + 1), packet.addr->sin_addr.s_addr,
           packet.addr->sin_port);
  }
  // send_to_socket(manager->fd, packet.buffer, packet.size, packet.addr);
  // return;

  if (send_and_wait_for_ack(manager, packet, seqnumber) < 0) {
    printf("couldn't send packet");
  }
  free(new_buffer);
  return seqnumber;
}

Packet *app_recv(socket_manager *manager) {
  sem_wait(manager->recieve_notify);

  Packet *packet = (Packet *)lpop_linklist(manager->recieve_buffer);
  // printf("READ2 %d %p %p\n", manager->recieve_buffer->size,
  // manager->recieve_buffer->root, manager->recieve_buffer->end);

  if (packet == NULL) {
    printf("recieved packet is null!, exiting");
    exit(1);
  }
  return packet;
}

int run_recv_daemon(socket_manager *manager) {
  char buf[DG_MAXSIZE];
  struct sockaddr_in client_addr;

  while (1) {
    int recv_len = recv_from_socket(manager->fd, buf, DG_MAXSIZE, &client_addr);

    if (recv_len < 0) {
      printf("can't recv data");
      exit(EXIT_FAILURE);
    }

    Packet packet = newPacket(&client_addr, buf, recv_len);
    unsigned char packet_type = *((unsigned char *)buf);

    printf("daemon recieved %u %hu from %d:%d\n", packet_type, *(buf + 1),
           client_addr.sin_addr.s_addr, client_addr.sin_port);
    switch (packet_type) {
    case 0:
      if (handle_recieved_app_packet(manager, packet)) {
        printf("couldn't process app packet");
        exit(EXIT_FAILURE);
      }

      break;
    case 1:

      if (handle_recieved_ack_packet(manager, packet)) {
        printf("couldn't process ack packet");
        exit(EXIT_FAILURE);
      }
      break;
    default:
      printf("unknown packet received");
      exit(EXIT_FAILURE);
    }
  }
}

void *run_recv_daemon_async(void *arg) {
  socket_manager *manager =
      (socket_manager *)arg; // Cast argument back to socket_manager type
  int result = run_recv_daemon(manager);
  return (void *)(intptr_t)result; // Return result as void pointer
}

void destroy_socket_manager(socket_manager *manager) {
  manager->running = 0;
  sleep(CONN_TEAR_DOWN_SLEEP);

  pthread_cancel(manager->recvDaemonID);

  destroy_hashmap(manager->ack_map);

  lock_rwmutex(manager->recieve_buffer->mutex);
  linklist_entry *iterator = manager->recieve_buffer->root;
  while (iterator != NULL) {
    Packet *packet = iterator->data;
    destroy_packet(*packet);
    iterator = iterator->next;
  }
  unlock_rwmutex(manager->recieve_buffer->mutex);

  destroy_linklist(manager->recieve_buffer);

  sem_destroy(manager->recieve_notify);
  free(manager->recieve_notify);
  pthread_mutex_destroy(manager->seqlock);
  free(manager->seqlock);

  close(manager->fd);
  free(manager);
}
/*
protocol info:
the first byte is packet type. it's either 0 which means app data or 1 which
means ACK the next two bytes are the seq number, either acking or need to when
acked.
*/
