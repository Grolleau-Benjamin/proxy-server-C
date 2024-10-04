#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>

#define BUFFER_SIZE 4096

typedef struct {
  int client_fd;
  int server_fd;
  char client_buffer[BUFFER_SIZE];
  char server_buffer[BUFFER_SIZE];
  ssize_t client_buffer_len;
  ssize_t server_buffer_len;
} connection_t;

int is_http_method(const char* buffer);
int is_http_request_complete(const char* buffer);
int init_listen_socket(const char* address, int port, int max_client);
int accept_connection(int listen_fd, struct sockaddr_in* client_addr);
int handle_connection(connection_t* conn);
void handle_http(connection_t* conn);

#endif
