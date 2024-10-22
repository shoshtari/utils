#include "socket.h"

int send_to_socket(int fd, char *data, int n, struct sockaddr_in *addr) {
  struct sockaddr *addr_to_use = (struct sockaddr *)addr;
  socklen_t addr_len = sizeof(*addr);

  for (int i = 0; i < MAX_RETRY; i++) {
    if (sendto(fd, data, n, 0, addr_to_use, addr_len) == -1) {
      perror("WARN couldn't send data to socket, retrying...");
      usleep(RETRY_SLEEP);
      continue;
    }
    return 0;
  }
  perror("ERROR couldn't send data to socket, retrying...");
  return -1;
}

int recv_from_socket(int fd, char *data, int n, struct sockaddr_in *addr) {
  struct sockaddr *addr_to_use = (struct sockaddr *)addr;
  socklen_t addr_len = sizeof(*addr);

  int recv_len;
  for (int i = 0; i < MAX_RETRY; i++) {
    if ((recv_len = recvfrom(fd, data, n, 0, addr_to_use, &addr_len)) < 0) {
      perror("WARN couldn't recieve data from client");
      usleep(RETRY_SLEEP);
      continue;
    }

    return recv_len;
  }
  perror("ERROR couldn't recieve data from client");

  return -1;
}


