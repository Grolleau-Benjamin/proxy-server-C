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
#include "../includes/server_helper.h"
#include "../includes/http_helper.h"
#include "../includes/dns_helper.h"
#include <stdio.h>

int init_listen_socket(const char* address, int port, int max_client) {
  int listen_fd, ret;
  struct sockaddr_in server_addr;
  
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) {
      ERROR("ERROR while creating socket\n");
      return -1;
  }
  INFO("Socket created (fd: %d)\n", listen_fd);
  Log(LOG_LEVEL_INFO, "Socket created (fd: %d)", listen_fd);
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_aton(address, &server_addr.sin_addr);
  ret = bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if (ret < 0) {
      perror("bind");
      return -1;
  }
  INFO("Server is bind on %s:%d\n", address, port);
  Log(LOG_LEVEL_INFO, "Server is bind on %s:%d", address, port);

  ret = listen(listen_fd, max_client);
  if (ret < 0) {
      perror("listen");
      return -1;
  }
  INFO("Server is now listening...\n");
  Log(LOG_LEVEL_INFO, "Server is now listening");

  return listen_fd;
}

int accept_connection(int listen_fd, struct sockaddr_in* client_addr, char* client_ip, int max_client, int nb_client) {
    socklen_t addr_len = sizeof(struct sockaddr_in);
    
    if (nb_client >= max_client) {
        INFO("Maximum number of clients reached. Connection refused.\n");
        return -1;
    }
    
    int new_client_fd = accept(listen_fd, (struct sockaddr*)client_addr, &addr_len);
    if (new_client_fd < 0) {
        print_error(new_client_fd, "accept");
        return -1;
    }

    INFO("New connection accepted: fd %d\n", new_client_fd);
    Log(LOG_LEVEL_INFO, "New client connection accepted: fd %d", new_client_fd);

    inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    INFO("Client IP: %s\n", client_ip);

    return new_client_fd;
}

int handle_connection(connection_t* conn) {
  INFO("Handling connection...\n");

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
      Log(LOG_LEVEL_INFO, "Client %d closed the connection", conn->client_fd);
      return 1;
    }

    total_bytes_read += bytes_read;
    conn->client_buffer_len = total_bytes_read;
    
    if (is_http_method(conn->client_buffer) && is_http_request_complete(conn->client_buffer)) {
      int ret = handle_http(conn);
      if (ret == 1) { return 1; }
      memset(conn->client_buffer, 0, sizeof(conn->client_buffer));
      conn->client_buffer_len = 0;
      total_bytes_read = 0;
      return 0;
    }

    if (total_bytes_read == BUFFER_SIZE) {
      if (!is_http_method(conn->client_buffer)) {
        WARN("Unknown protocol.\n");
        Log(LOG_LEVEL_WARN, "Client have write %d bytes and http havn't been recognize...", total_bytes_read);
      } else {
        Log(LOG_LEVEL_WARN, "This http request is to huge to handle.");
        WARN("Request too large to handle.\n");
      }
      return 1;
    }
  }
  return 0;
}

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

    char* ip = NULL;
    char* port = NULL;
    int sockfd = -1;
    struct addrinfo *res = NULL;

    // handle the case where client ask for GET http://localhost:port/item HTTP/1.1
    // serveur return 404 NOT FOUND, so we have to change the request to : GET /item HTTP/1.1
    if (replace_localhost_with_ip(host)) {
        char* get_pos = strstr(conn->client_buffer, "GET http://localhost");
        if (get_pos) {
            char* path_start = strchr(get_pos, '/');  // Find the first /
            if (path_start) {
                path_start = strchr(path_start + 2, '/');  // go to the last /
                if (path_start) {
                    char* http_version = strstr(path_start, " HTTP/1.1"); // got to the end (start of HTTP version protocol)
                    if (http_version) {
                        char new_line[1024];
                        snprintf(new_line, sizeof(new_line), "GET %.*s HTTP/1.1\r\n", (int)(http_version - path_start), path_start); // write the new line;

                        // find fisrt end of line
                        char* end_of_line = strstr(conn->client_buffer, "\r\n");
                        if (end_of_line) {
                            memmove(get_pos, new_line, strlen(new_line)); // replace the line by new line

                            // adjust the rest of the buffer
                            // get_pos + strlen(new_line) => end of the line
                            // end_of_line + 2 => for the \r\n
                            // strlen(end_of_line + 2) +1 => to copy the rest of the buffer + 1 (\0)
                            memmove(get_pos + strlen(new_line), end_of_line + 2, strlen(end_of_line + 2) + 1);
                        }
                    }
                }
            }
        }
        INFO("Localhost case handled\n");
    }

    if (is_host_https_format(host)) {
        WARN("https format for %s\n", host);
        WARN("Sending a 404 not found\n");
        write_on_socket_http_from_buffer(conn->client_fd, HTTP_404_RESPONSE, sizeof(HTTP_404_RESPONSE));
        return 1;
    }
    // If the format of host is IP:Port, we don't solve the DNS and connect
    else if (is_ip_port_format(host, &ip, &port)) {
        INFO("is_ip_port_format\n");
        INFO("IP: %s\n\tPort: %s\n", ip, port);

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(atoi(port));

        if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
            ERROR("Invalid IP address");
            return 1;
        }

        // Creating the server socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            ERROR("server socket creation failed");
            return 3;
        }

        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
            ERROR("Failed to connect");
            close(sockfd);
            return 4;
        }

        INFO("Connected to %s on port %s\n", ip, port);
        Log(LOG_LEVEL_INFO, "Connected to %s on port %s", ip, port);
    } else {
        // Else: DNS resolution and connection
        int status;
        char ipstr[INET6_ADDRSTRLEN];

        if (resolve_dns(host, &res, ipstr) != 0) {
            return 2; 
        }

        // Creating the socket
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd == -1) {
            ERROR("dns socket creation failed");
            freeaddrinfo(res);
            return 3;
        }

        if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
            ERROR("Failed to connect");
            close(sockfd);
            freeaddrinfo(res);
            return 4;
        }

        INFO("Connected to %s on port 80\n", ipstr);
        Log(LOG_LEVEL_INFO, "Connected to %s on port 80\n", ipstr);
    }

    // Writing the client's buffer on socker
    if (write_on_socket_http_from_buffer(sockfd, conn->client_buffer, conn->client_buffer_len) != 0) {
        ERROR("Error while writing on the socket to the host %s, IP %s", host, conn->server_ip);
        Log(LOG_LEVEL_ERROR, "Error while writing on the socket to the host %s, IP %s", host, conn->server_ip);

        close(sockfd);
        return 1;
    } else {
        INFO("Writing OK\n");
        Log(LOG_LEVEL_INFO, "Client %d have write %d bytes to server", conn->client_fd, conn->client_buffer_len);
    }

    conn->server_fd = sockfd;

    INFO("End of handle_http\n");
    return 0;
}
