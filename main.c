#include "includes/utils.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>

#define MAX_CLIENT 10
#define PORT 8080
#define ADDRESS "127.0.0.1"

int main() {
  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  print_error(listen_fd, "socket");
  INFO("Socket created (fd: %d)\n", listen_fd);
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  inet_aton(ADDRESS, &server_addr.sin_addr);
  int ret = bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  print_error(ret, "bind");
  INFO("Server is bind on %s:%d\n", ADDRESS, PORT);
  
  close(listen_fd);
  return 0;
}
