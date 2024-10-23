

#ifndef NET_SOCKET_INCLUDE
#define NET_SOCKET_INCLUDE

#include <netinet/in.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "pthread.h"

#define SEND_TIMEOUT 5000000
#define RETRY_SLEEP 20000
#define CHECK_SLEEP 1000
#define CONN_TEAR_DOWN_SLEEP 3

int send_to_socket(int fd, char *data, int n, struct sockaddr_in *addr);
int recv_from_socket(int fd, char *data, int n, struct sockaddr_in *addr);
#endif
