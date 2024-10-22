
#ifndef NET_SOCKET_INCLUDE 
#define NET_SOCKET_INCLUDE 

#include "pthread.h"
#include <netinet/in.h>
#include <semaphore.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_RETRY 5
#define RETRY_SLEEP 500000

int send_to_socket(int fd, char *data, int n, struct sockaddr_in *addr) ;
int recv_from_socket(int fd, char *data, int n, struct sockaddr_in *addr) ;
#endif
