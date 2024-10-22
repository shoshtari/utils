#include "socket.h"

int send_to_socket(int fd, char *data, int n, struct sockaddr_in *addr) {
  struct sockaddr *addr_to_use = (struct sockaddr *)addr;
  socklen_t addr_len = sizeof(*addr);

  if (sendto(fd, data, n, 0, addr_to_use, addr_len) == -1) {
    perror("WARN couldn't send data to socket, retrying...");
    usleep(RETRY_SLEEP);
    return -1;
  }
  return 0;
}

int recv_from_socket(int fd, char *data, int n, struct sockaddr_in *addr) {
  struct sockaddr *addr_to_use = (struct sockaddr *)addr;
  socklen_t addr_len = sizeof(*addr);

  return recvfrom(fd, data, n, 0, addr_to_use, &addr_len);
}
