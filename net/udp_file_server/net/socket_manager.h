#ifndef SOCKET_MANAGER_INC
#define SOCKET_MANAGER_INC

#include <netinet/in.h>
#include <semaphore.h>
#include <string.h>

#include "../data_structures/hashmap.h"
#include "../data_structures/linklist.h"
#include "pthread.h"

#define ACKMAP_CARDINALITY 1024
#define PROTOCOL_OVERHEAD 3
#define DG_MAXSIZE 60000

typedef struct Packet {
    uint32_t size;
    char *buffer;
    struct sockaddr_in *addr;
} Packet;

typedef struct socket_manager {
    hashmap *ack_map;          // a map from addr-seqnumber combination to a mutex
    linklist *recieve_buffer;  // a linklist of mutexes, when a packet receive one

    sem_t *recieve_notify;

    unsigned short seqnumber;
    pthread_mutex_t *seqlock;

    int fd;

} socket_manager;

socket_manager *new_socket_manager(int socket_fd);
Packet newPacket(struct sockaddr_in *addr, char *buf, int bufferSize);

void app_send(socket_manager *manager, Packet packet);
Packet *app_recv(socket_manager *manager);
int run_recv_daemon(socket_manager *manager);
void *run_recv_daemon_async(void *arg);
void destroy_socket_manager(socket_manager *manager);
#endif
