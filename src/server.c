/*
 * MIT License
 * 
 * Copyright (c) 2024
 * Benjamin Grolleau
 * Alexis Carle
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction.
 * 
 * See the LICENSE file for the full license text.
 */

#include "../includes/server.h"
#include "../includes/utils.h"
#include "../includes/logger.h"
#include <arpa/inet.h>

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

int accept_connection(int listen_fd, struct sockaddr_in* client_addr, char* client_ip) {
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int new_client_fd = accept(listen_fd, (struct sockaddr*)client_addr, &addr_len);
    print_error(new_client_fd, "accept");
    INFO("New connection accepted: fd %d\n", new_client_fd);

    // Get the IPv4 address as a string
    inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    INFO("Client IP: %s\n", client_ip);

    return new_client_fd;
}

int handle_connection(connection_t* conn) {
  INFO("Handle connection function\n");

  ssize_t total_bytes_read = 0;
  conn->client_buffer_len = 0;

  while(1) {
    ssize_t bytes_read = read(conn->client_fd, conn->client_buffer + total_bytes_read, sizeof(conn->client_buffer) - total_bytes_read);

    if (bytes_read < 0) {
      print_error(bytes_read, "read");
      return 1;
    }

    if (bytes_read == 0) {
      INFO("Client closed the connection.\n");
      return 1;
    }

    total_bytes_read += bytes_read;
    conn->client_buffer_len = total_bytes_read;
    
    if (is_http_method(conn->client_buffer) && is_http_request_complete(conn->client_buffer)) {
      int ret = handle_http(conn);
      if (ret == -1) { return 1; }
      memset(conn->client_buffer, 0, sizeof(conn->client_buffer));
      conn->client_buffer_len = 0;
      total_bytes_read = 0;
    }

    if (total_bytes_read == BUFFER_SIZE) {
      if (!is_http_method(conn->client_buffer)) {
        WARN("Unknown protocol.\n");
      } else {
        WARN("Request too large to handle.\n");
      }
      return 1;
    }
  }
  return 0;
}

// TODO: write HTTP'404 request before return 1 and closing the socket.
int handle_http(connection_t* conn) {
  INFO("Handle HTTP function\n");
  
  char host[256] = {0};
  if (get_http_host(conn->client_buffer, host, sizeof(host)) == 0) {
    INFO("Host: %s\n", host);
  } else {
    WARN("Failed to retrieve host from request.\n");
    Log(LOG_LEVEL_WARN, "%s made a request without a valid Host header.", conn->client_ip);
    return 1;
  }

  Log(LOG_LEVEL_INFO, "%s asked for %s", conn->client_ip, host);

  // TODO: Host and buffer filter

  struct hostent *he = gethostbyname(host);
  if (he == NULL) {
    perror("gethostbyname");
    ERROR("Error while solving host %s for the client client %s\n", host, conn->client_ip);
    Log(LOG_LEVEL_ERROR, "Error while solving host %s for the client client %s", host, conn->client_ip);
    return 1;
  }  

  char ip[INET_ADDRSTRLEN];
  if (inet_ntop(AF_INET, he->h_addr, ip, sizeof(ip)) == NULL) {
    perror("inet_ntop");
    ERROR("Error while converting %s's ip to writable string\n", host); 
    return 1;
  }
  INFO("Host %s have IPv4 %s\n", host, ip);
  
  return 0;
}
