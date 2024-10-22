#include "data_structures/hashmap.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "file_utils.h"
#include "net/socket_manager.h"
#include "print_utils.h"

#define SERVER "127.0.0.1" // Server IP address
#define TARGET_DIR "./files/"
#define RESPONSE_TIMEOUT 5000000
#define RESPONSE_WAIT_TIME 100000

struct sockaddr_in server_address;
unsigned int server_length = sizeof(struct sockaddr);
socket_manager *manager;

typedef struct RecieveHookEntry {
  int ready;
  void *buffer;
  int buffersize;
} RecieveHookEntry;
hashmap *receiveHooks;

int compare_short(void *a, void *b) {
  return *(unsigned short *)a - *(unsigned short *)b;
}
uint32_t hash_short(void *a, int size) { return *(unsigned short *)a; }

int conn_ready = 0;
#define HOOK_CARDINALITY 1024
pthread_mutex_t hookLock;

void upsertHook(unsigned short seqnumber, int ready, void *buffer,
                int buffersize) {
  RecieveHookEntry entry;
  entry.ready = ready;
  entry.buffer = buffer;
  entry.buffersize = buffersize;

  pthread_mutex_lock(&hookLock);
  if (hashmap_get(receiveHooks, &seqnumber) != NULL && ready == 0) {
    // if already exists, then it updates only if ready is 1 (it doesn't
    // recreate or reverse ready to 0

    pthread_mutex_unlock(&hookLock);
    return;
  }
  hashmap_set(receiveHooks, &seqnumber, &entry);

  pthread_mutex_unlock(&hookLock);
}

void *RecieveHandler(void *args) {
  pthread_mutex_init(&hookLock, NULL);
  receiveHooks =
      create_hashsmap(HOOK_CARDINALITY, sizeof(unsigned short),
                      sizeof(RecieveHookEntry), compare_short, hash_short);
  conn_ready = 1;
  while (1) {
    Packet *packet = app_recv(manager);
    if (packet == NULL) {
      printf("Empty packet recieved!\n");
      continue;
    }
	if (packet->size < PROTOCOL_OVERHEAD + 2){
		printf("packet size is need to be at least %d\n", PROTOCOL_OVERHEAD + 2);
		continue;
	}

    unsigned short seqnumber = *((unsigned short *)(packet->buffer + 3));
    upsertHook(seqnumber, 1, packet->buffer, packet->size);
    free(packet->addr);
  }
}
RecieveHookEntry waitForRes(unsigned short seqnumber) {

  unsigned int waitTime = 0;
  upsertHook(seqnumber, 0, NULL, 0);

  while (1) {
    RecieveHookEntry *entry =
        (RecieveHookEntry *)hashmap_get(receiveHooks, &seqnumber);
    if (entry == NULL) {
      printf("hashmap sucks!\n");
      exit(1);
    }
    if (entry->ready) {
      return *entry;
      break;
    }
    usleep(RESPONSE_WAIT_TIME);
    waitTime += RESPONSE_WAIT_TIME;
    if (waitTime > RESPONSE_TIMEOUT) {
      printf("timeout reached to get response for seq %d\n", seqnumber);
      exit(1);
    }
  }
}

int rid = 0;
int ping() {
  char *message = "ping";

  unsigned short seqnumber = app_send(
      manager, newPacket(&server_address, message, strlen(message) + 1));

  if (seqnumber == 0) {
    printf("invalid seqnumber %hu!\n", seqnumber);
    return -1;
  }

  RecieveHookEntry entry = waitForRes(seqnumber);

  printf("Received reply: ");
  try_print(entry.buffer + PROTOCOL_OVERHEAD + 2,
            entry.buffersize - PROTOCOL_OVERHEAD - 2);
  free(entry.buffer);

  return 0;
}

int list_files(dir_files *result) {
  int limit = 100;
  int capacity = 1;
  result->filecounts = 0;
  result->files = malloc(sizeof(fileinfo));

  char *message = malloc(DG_MAXSIZE);
  while (1) {
    sprintf(message, "list-%d-%d", limit, result->filecounts);

    unsigned short seqnumber = app_send(
        manager, newPacket(&server_address, message, strlen(message) + 1));

    RecieveHookEntry entry = waitForRes(seqnumber);

    char *recievedData = entry.buffer + PROTOCOL_OVERHEAD + 2;
    printf("data  %hu recieved ", seqnumber);
    try_print(recievedData, entry.buffersize - PROTOCOL_OVERHEAD - 2);


    if (strlen(recievedData) == 0) {
      break;
    }

    char *strtok_buf;
    char *file_entry = strtok_r(recievedData, ";", &strtok_buf);

    while (file_entry != NULL) {
      if (strlen(file_entry) == 0) {
        file_entry = strtok_r(NULL, ";", &strtok_buf);
        continue;
      }

      if (result->filecounts == capacity) {
        result->files = (fileinfo *)realloc_pointer(
            (char *)result->files, result->filecounts * sizeof(fileinfo));
        capacity *= 2;
      }
      char *parts = strtok(file_entry, "@"); // id
      int id = atoi(parts);
      if (id != result->filecounts) {
        printf(
            "id %d is not equal to count %d, this shouldn't happen normally\n",
            id, result->filecounts);
      }

      fileinfo file;
      parts = strtok(NULL, "@"); // filename
      file.name = malloc(strlen(parts) + 10);
      strcpy(file.name, parts);

      parts = strtok(NULL, "@"); // filesize
      file.size = atoi(parts);
      file.data = malloc(file.size);

      parts = strtok(NULL, "@"); // filehash
      file.hash = malloc(strlen(parts) + 10);
      strcpy(file.hash, parts);

      file.fd = -1;
      result->files[result->filecounts] = file;
      result->filecounts++;

      file_entry = strtok_r(NULL, ";", &strtok_buf);
    }
  }
  free(message);
  return 0;
}

int get_file(dir_files files, int fileid) {
  if (files.filecounts == 0) {
    printf("file list is empty, getting list\n");
    if (list_files(&files)) {
      printf("couldn't get file list");
      return -1;
    }
  }

  fileinfo file = files.files[fileid];
  printf("downloding %s %d\n", file.name, fileid);
  char *res = malloc(file.size);
  char *message = malloc(DG_MAXSIZE);
  int chunksize = 50000;
  for (int i = 0; i < file.size; i += chunksize) {
    int chunk_size = i + chunksize > file.size ? file.size - i : chunksize;
    sprintf(message, "get-%d-%d-%d", fileid, i, chunk_size);
    unsigned short seqnumber =
        app_send(manager, newPacket(&server_address, message, strlen(message)));

    RecieveHookEntry entry = waitForRes(seqnumber);

    memcpy(res + i, entry.buffer + PROTOCOL_OVERHEAD  + 2, chunk_size);
  }

  printf("%s %d %d\n", file.name, file.size, fileid);
  char *target_file = (char *)malloc(FILENAME_MAX);
  strcpy(target_file, TARGET_DIR);
  strcat(target_file, file.name);

  if (write_to_file(target_file, res, file.size) < 0) {
    printf("couldn't write to file");
    return -1;
  };

  free(message);
  free(target_file);
  free(res);
  printf("file %s downloaded\n", file.name);

  return 0;
}

void menu() {
  int action;
  int number;

  while (1) {
    // Display the menu
    printf("Menu:\n");
    printf("1. ping\n");
    printf("2. list\n");
    printf("3. get NUMBER\n");
    printf("4. exit\n");
    printf("Please enter the action number (1-4): ");

    // Get user input
    if (scanf("%d", &action) != 1) {
      printf("Invalid input. Please enter a number between 1 and 4.\n");
      // Clear input buffer
      while (getchar() != '\n')
        ;
      continue;
    }

    dir_files files;
    list_files(&files);
    // Handle the selected action
    switch (action) {
    case 1:
      ping();
      break;
    case 2:
      free_file(files);
      list_files(&files);
      print_files(files);
      break;
    case 3:
      printf("Please enter a number: ");
      if (scanf("%d", &number) == 1) {
        printf("You selected get %d: Dummy text for getting number %d.\n",
               number, number);
      } else {
        printf("Invalid input for number. Please enter an integer.\n");
        // Clear input buffer if invalid
        while (getchar() != '\n')
          ;
      }
      break;
    case 4:
      printf("Exiting the menu. Goodbye!\n");
      return; // Exit the loop
    default:
      printf("Invalid action number. Please select a valid option (1-4).\n");
      break;
    }
    printf("Press Enter to continue...\n");

    char buffer[10];
    fgets(buffer, sizeof(buffer), stdin);
    fgets(buffer, sizeof(buffer), stdin);
  }
}

int op(int port) {
  printf("running client...\n");

  // Create a UDP socket
  int sockfd;
  sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sockfd < 0) {
    printf("socket");
    exit(EXIT_FAILURE);
  }

  // struct timeval tv;
  // tv.tv_sec = 0;
  // tv.tv_usec = (int)1e2;
  // if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
  //     printf("can't set timeout for recieve");
  // }

  memset((char *)&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);

  if (inet_aton(SERVER, &server_address.sin_addr) == 0) {
    fprintf(stderr, "inet_aton() failed\n");
    exit(EXIT_FAILURE);
  }

  manager = new_socket_manager(sockfd);
  if (pthread_create(&manager->recvDaemonID, NULL, run_recv_daemon_async,
                     (void *)manager) != 0) {
    printf("Failed to create thread");
    return 1;
  }

  pthread_t pid;
  if (pthread_create(&pid, NULL, RecieveHandler, NULL) != 0) {
    printf("Failed to create thread");
    return 1;
  }

  while (!conn_ready) {
    usleep(10000);
  }

  printf("connection is ready!\n");
  // connection is ready
  dir_files files;
  ping();
  printf("######################### ping done ##################\n");
  list_files(&files);
  free_file(files);
  list_files(&files);
  print_files(files);
  printf("######################### list done ##################\n");
  get_file(files, 0);
  free_file(files);

  app_send(manager, newPacket(&server_address, "exit", 5));

  destroy_socket_manager(manager);

  printf("Bye!\n");
  return 0;
}

int main(int argc, char **argv) {
  sleep(1);
  if (argc != 2) {
    printf("you must run this like ./server PORT");
    exit(EXIT_FAILURE);
  }

  unsigned int port = atoi(argv[1]);
  op(port);
  return 0;
}
