#include "socket_manager.h"

#include <pthread.h>

#include "socket.h"

typedef struct ackmap_key {
    unsigned short int seqnumber;
    struct sockaddr_in addr;
} ackmap_key;

socket_manager *new_socket_manager(int socket_fd) {
    socket_manager *manager = malloc(sizeof(socket_manager));
    manager->ack_map = create_hashsmap(ACKMAP_CARDINALITY, sizeof(ackmap_key),
                                       sizeof(pthread_mutex_t));
    manager->recieve_buffer = create_linklist(sizeof(Packet));
    manager->recieve_notify = malloc(sizeof(sem_t));
    sem_init(manager->recieve_notify, 1, 0);

    manager->seqlock = malloc(sizeof(pthread_mutex_t));

    pthread_mutex_init(manager->seqlock, NULL);
    manager->seqnumber = 1;
    manager->fd = socket_fd;
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

int send_and_wait_for_ack(socket_manager *manager, Packet packet,
                          unsigned short seqnumber) {
    ackmap_key k;
    k.addr = *packet.addr;
    k.seqnumber = seqnumber;

    pthread_mutex_t lock;
    printf("creating lock %p\n", &lock);
    if (pthread_mutex_init(&lock, NULL) < 0) {
        perror("couldn't initialize mutex");
        return -1;
    }

    hashmap_set(manager->ack_map, &k, &lock);
    pthread_mutex_destroy(&lock);

    if (pthread_mutex_lock(hashmap_get(manager->ack_map, &k)) < 0) {
        perror("ack first lock err");
        return -1;
    }

    if (send_to_socket(manager->fd, packet.buffer, packet.size, packet.addr) < 0) {
        perror("couldn't send data to socket");
        return -1;
    }

    printf("returning from send1\n");
    if (pthread_mutex_lock(hashmap_get(manager->ack_map, &k)) < 0) {
        perror("ack second lock err");
        return -1;
    }
    printf("returning from send2\n");
    pthread_mutex_destroy(hashmap_get(manager->ack_map, &k));

    hashmap_del(manager->ack_map, &k);
    return 0;
}

int send_ack(socket_manager *manager, char *received_buf,
             struct sockaddr_in *addr) {
    unsigned short seqnumber = *((unsigned short *)(received_buf + 1));
    char *buf = malloc(PROTOCOL_OVERHEAD);
    set_protocol_headers(manager, buf, seqnumber);

    return send_to_socket(manager->fd, buf, PROTOCOL_OVERHEAD, addr);
}

int handle_recieved_app_packet(socket_manager *manager, Packet packet) {
    if (send_ack(manager, packet.buffer, packet.addr) < 0) {
        perror("couldn't send ack");
        return -1;
    }

    rinsert_linklist(manager->recieve_buffer, &packet);

    sem_post(manager->recieve_notify);
    return 0;
}
int handle_recieved_ack_packet(socket_manager *manager, Packet packet) {
    ackmap_key key;
    key.addr = *packet.addr;
    key.seqnumber = *((unsigned short *)(packet.buffer + 1));

    pthread_mutex_t *lock = hashmap_get(manager->ack_map, &key);

    if (lock == NULL) {
        return 0;
    }

    printf("releasing %d %p\n", key.seqnumber, lock);
    return pthread_mutex_unlock(lock);
}

void app_send(socket_manager *manager, Packet packet) {
    printf("%d\n", packet.size);
    if (packet.size + PROTOCOL_OVERHEAD > DG_MAXSIZE) {
        perror("packet too big");
        return;
    }

    char *new_buffer = malloc(packet.size + PROTOCOL_OVERHEAD);
    unsigned short seqnumber = set_protocol_headers(manager, new_buffer, 0);
    memcpy(new_buffer + PROTOCOL_OVERHEAD, packet.buffer, packet.size);
    packet.buffer = new_buffer;
    packet.size += PROTOCOL_OVERHEAD;

    printf("sending: %u %hu %s\n", packet.buffer[0], *(packet.buffer + 1), packet.buffer + 3);
    // send_to_socket(manager->fd, packet.buffer, packet.size, packet.addr);
    // return;

    if (send_and_wait_for_ack(manager, packet, seqnumber) < 0) {
        perror("couldn't send packet");
    }
}

Packet *app_recv(socket_manager *manager) {
    sem_wait(manager->recieve_notify);

    Packet *packet = (Packet *)lpop_linklist(manager->recieve_buffer);

    return packet;
}

int run_recv_daemon(socket_manager *manager) {
    char buf[DG_MAXSIZE];
    struct sockaddr_in client_addr;

    while (1) {
        int recv_len = recv_from_socket(manager->fd, buf, DG_MAXSIZE, &client_addr);

        if (recv_len < 0) {
            perror("can't recv data");
            exit(EXIT_FAILURE);
        }

        printf("daemon recieved %u %hu %s\n", buf[0], *(buf + 1), buf + 3);
        Packet packet = newPacket(&client_addr, buf, recv_len);
        unsigned char packet_type = *((unsigned char *)buf);
        switch (packet_type) {
            case 0:
                printf("App Data\n");
                if (handle_recieved_app_packet(manager, packet)) {
                    perror("couldn't process app packet");
                    return -1;
                }

                break;
            case 1:

                if (handle_recieved_ack_packet(manager, packet)) {
                    perror("couldn't process ack packet");
                    return -1;
                }
                printf("Done\n");
                break;
            default:
                perror("unknown package recieved");
        }
    }
}

void *run_recv_daemon_async(void *arg) {
    socket_manager *manager = (socket_manager *)arg;  // Cast argument back to socket_manager type
    int result = run_recv_daemon(manager);
    return (void *)(intptr_t)result;  // Return result as void pointer
}

void destroy_socket_manager(socket_manager *manager) {
    destroy_hashmap(manager->ack_map);
    destroy_linklist(manager->recieve_buffer);
    free(manager);
}
/*
protocol info:
the first byte is packet type. it's either 0 which means app data or 1 which
means ACK the next two bytes are the seq number, either acking or need to when
acked.
*/
