#include "../includes/server.h"
#include "../includes/utils.h"
#include <sys/types.h>

const char* http_methods[] = {
  "GET", "POST", "HEAD", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"
};

const char* ftp_commands[] = {
  "USER", "PASS", "ACCT", "CWD", "QUIT", "REIN", "PORT", "PASV", "TYPE", "MODE", "STRU", "RETR", "STOR"
};

int is_http_method(const char* buffer) {
  for(size_t i = 0; i < sizeof(http_methods) / sizeof(http_methods[0]); i++) {
    if (strncmp(buffer, http_methods[i], strlen(http_methods[i])) == 0) {
      return 1;
    }
  }
  return 0;
}

int is_http_request_complete(const char* buffer) {
  const char* end_of_headers = "\r\n\r\n";
  if (strstr(buffer, end_of_headers) != NULL) {
    return 1;
  }
  return 0;
}

int is_ftp_command(const char *buffer) {
  for (size_t i = 0; i < sizeof(ftp_commands) / sizeof(ftp_commands[0]); i++) {
    if (strncmp(buffer, ftp_commands[i], strlen(ftp_commands[i])) == 0) {
      return 1;
    }
  }
  return 0;
}

int is_ftp_command_complete(const char *buffer) {
  const char *end_of_command = "\r\n";
  if (strstr(buffer, end_of_command) != NULL) {
    return 1; 
  }
  return 0; 
}

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

int handle_connection(connection_t* conn) {
  INFO("Handle connection function\n");

  char buffer[BUFFER_SIZE];
  ssize_t total_bytes_read = 0;

  while(1) {
    ssize_t bytes_read = read(conn->client_fd, buffer + total_bytes_read, sizeof(buffer) - total_bytes_read);

    if (bytes_read < 0) {
      print_error(bytes_read, "read");
      return 1;
    }

    if (bytes_read == 0) {
      INFO("Client closed the connection.\n");
      return 1;
    }

    total_bytes_read += bytes_read;
    
    if (is_http_method(buffer) && is_http_request_complete(buffer)) {
      handle_http(conn, buffer, total_bytes_read);
      break;
    } else if (is_ftp_command(buffer) && is_ftp_command_complete(buffer)) {
      handle_ftp(conn, buffer, total_bytes_read);
      break;
    }

    if (total_bytes_read == BUFFER_SIZE) {
      if (!(is_ftp_command(buffer) || is_http_method(buffer))) {
        WARN("Unknown protocol.\n");
      } else {
        WARN("Request too large to handle.\n");
      }
      return 1;
    }
  }
  return 0;
}

void handle_ftp(connection_t* conn, const char* buffer, ssize_t buffer_len) {
  INFO("Handle FTP function\n");  
  INFO("Command: %.*s\n", (int)buffer_len, buffer);
}

void handle_http(connection_t* conn, const char* buffer, ssize_t buffer_len) {
  INFO("Handle HTTP function\n");
  INFO("Request: %.*s\n", (int)buffer_len, buffer);
}
