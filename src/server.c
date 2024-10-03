#include "../includes/server.h"
#include "../includes/utils.h"

int init_listen_socket(const char* address, int port, int max_client) {
  int listen_fd, ret;
  struct sockaddr_in server_addr;
  
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  print_error(listen_fd, "socket");
  INFO("Socket created (fd: %d)\n", listen_fd);
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_aton(address, &server_addr.sin_addr);
  ret = bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  print_error(ret, "bind");
  INFO("Server is bind on %s:%d\n", address, port);

  ret = listen(listen_fd, max_client);
  print_error(ret, "listen");
  INFO("Server is now listening...\n");

  return listen_fd;
}

int accept_connection(int listen_fd, struct sockaddr_in* client_addr) {
  socklen_t addr_len = sizeof(struct sockaddr_in);
  int new_client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len);
  print_error(new_client_fd, "accept");
  INFO("New connection accepted: fd %d\n", new_client_fd);
  return new_client_fd;
}
