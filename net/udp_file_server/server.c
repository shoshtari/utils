
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <dirent.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "file_utils.h"
#include "net/socket_manager.h"
#include "print_utils.h"

#define DIR_TO_SERVE "/home/morteza/Pictures/wallpapers"
#define LAZY_LOAD 1
#define POOL_SIZE 32
socket_manager *manager;

int running = 1;
char *gen_file_list(dir_files files, int limit, int offset) {
  int length = 10000;
  char *ans = malloc(length);
  int occupied = 0;

  limit += offset;
  for (int i = offset; i < files.filecounts && (i < limit || limit == -1);
       i++) {
    char *new_file_entry = gen_file_list_entry(files, i);
    if (new_file_entry == NULL) {
      printf("file entry is null\n");
      break;
    }
    int new_char_count = strlen(new_file_entry);
    if ((new_char_count + occupied) > DG_MAXSIZE) {
      break;
    }
    while (new_char_count + occupied + 10 >= length) {
      ans = realloc_pointer(ans, length);
      length *= 2;
    }

    memcpy(ans + occupied, new_file_entry, new_char_count);
    // strcat(ans, new_file_entry);
    occupied += strlen(new_file_entry);
    free(new_file_entry);
  }
  ans[occupied] = '\0';

  printf("%s\n", ans);
  return ans;
}

void *make_server_packet(void *buffer, int bufferSize,
                         unsigned short clientSeqnumber) {
  void *newBuffer = malloc(bufferSize + 2);
  memcpy(newBuffer, &clientSeqnumber, 2);
  memcpy(newBuffer + 2, buffer, bufferSize);
  return newBuffer;
}

sem_t poolSemaphore;
void serve(dir_files files) {


  Packet sendPacket;
  fflush(stdout);

  while (running) {
    Packet *receivedPacket = app_recv(manager);

    Packet sendPacket;

    unsigned short clientSeq =
        *((unsigned short *)(receivedPacket->buffer + 1));

    char *buf = receivedPacket->buffer + PROTOCOL_OVERHEAD;
    char *command = strtok(buf, "-");

    char *text = "unknown command";
    if (strcmp(command, "ping") == 0) {
      text = "pong";
      sendPacket = newPacket(receivedPacket->addr, text, strlen(text));
    } else if (strcmp(command, "list") == 0) {
      char *limitStr = strtok(NULL, "-");
      char *offsetStr = strtok(NULL, "-");
      int limit = atoi(limitStr);
      int offset = atoi(offsetStr);

      text = gen_file_list(files, limit, offset);
      sendPacket = newPacket(receivedPacket->addr, text, strlen(text) + 1);
      free(text);
    } else if (strcmp(command, "get") == 0) {

      char *tok = strtok(NULL, "-");
      if (tok == NULL) {
        printf("HOOOA %s\n", buf);
        printf("invalid get command. file not specified\n");
        exit(1);
      }
      int fileid = atoi(tok);

      tok = strtok(NULL, "-");
      if (tok == NULL) {
        printf("invalid get command. offset not specified\n");
        exit(1);
      }
      int start = atoi(tok);

      tok = strtok(NULL, "-");
      if (tok == NULL) {
        printf("invalid get command. size not specified\n");
        exit(1);
      }
      int size = atoi(tok);

      size = files.files[fileid].size < size ? files.files[fileid].size : size;

      printf("download file: %d from: %d size: %d\n", fileid, start, size);

      char *chunk;
      if (!LAZY_LOAD) {
        chunk = files.files[fileid].data + start;
      } else {
        chunk = malloc(size);
        read_chunk(files.files[fileid].fd, chunk, size, start);
      }
      sendPacket = newPacket(receivedPacket->addr, chunk, size);
    } else if (strcmp(command, "exit") == 0) {
      running = 0;
      destroy_packet(*receivedPacket);
      free(receivedPacket);
      sem_post(&poolSemaphore);
      return;
    }

    void *buffer =
        make_server_packet(sendPacket.buffer, sendPacket.size, clientSeq);
    free(sendPacket.buffer);
    sendPacket.buffer = buffer;
    sendPacket.size += 2;
    app_send(manager, sendPacket);
    printf("%hu served\n", clientSeq);
    destroy_packet(sendPacket);
    sem_post(&poolSemaphore);
  }
}

void *runWorker(void *args) {
  dir_files files = *((dir_files *)args);
  serve(files);
  return NULL;
}
int main(int argc, char **argv) {
  printf("running server...\n");

  if (argc != 2) {
    perror("you must run this like ./server PORT");
    exit(EXIT_FAILURE);
  }

  unsigned int port = atoi(argv[1]);
  int sockfd;
  // Create a UDP socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }
  struct sockaddr_in server_addr;

  // Zero out the structure
  memset((char *)&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  server_addr.sin_port = htons(port);

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int)) <
      0) {
    perror("can't set reuse address");
    exit(EXIT_FAILURE);
  }

  // Bind the socket to the port
  if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    perror("couldn't bind socket");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  dir_files files;

  read_files(DIR_TO_SERVE, &files, !LAZY_LOAD);
  print_files(files);

  manager = new_socket_manager(sockfd);
  if (pthread_create(&manager->recvDaemonID, NULL, run_recv_daemon_async,
                     (void *)manager) != 0) {
    perror("Failed to create thread");
    return 1;
  }

  printf("Server listening on port %d\n", port);

  printf("serving %d files\n", files.filecounts);
  pthread_t pids[POOL_SIZE];
  for (int i = 0; i < POOL_SIZE; i++) {
    pthread_create(&pids[i], NULL, runWorker, &files);
  }
  for (int i = 0; i < POOL_SIZE; i++) {
    pthread_join(pids[i], NULL);
  }
  // serve(files);

  destroy_socket_manager(manager);
  free_file(files);

  printf("Bye\n");
  return 0;
}
