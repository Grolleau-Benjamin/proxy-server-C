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

 /**
 * @file server.c
 * @brief This file contains the core server-side functions to handle network communication,
 * including socket initialization, connection acceptance, and HTTP request processing.
 */

#include "../includes/server.h"
#include "../includes/utils.h"
#include "../includes/logger.h"
#include "../includes/server_helper.h"
#include "../includes/http_helper.h"
#include "../includes/dns_helper.h"
#include "../includes/rules.h"
#include <netdb.h>
#include <stdio.h>

/**
 * @brief Initializes a listening socket.
 * 
 * This function creates a socket, binds it to the specified address and port, and starts listening for incoming connections.
 * 
 * @param address The IP address to bind the socket.
 * @param port The port number to bind the socket.
 * @param max_client The maximum number of clients the server can handle simultaneously.
 * 
 * @return The file descriptor of the listening socket, or -1 in case of an error.
 */
int init_listen_socket(const char* address, int port, int max_client) {
  int listen_fd, ret;
  struct sockaddr_in server_addr;
  
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) {
      ERROR("ERROR while creating socket\n");
      Log(LOG_LEVEL_ERROR, "[SERVER] ERROR while creating socket");
      return -1;
  }
  INFO("Socket created (fd: %d)\n", listen_fd);
  Log(LOG_LEVEL_INFO, "[SERVER] Socket created (fd: %d)", listen_fd);
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_aton(address, &server_addr.sin_addr);
  int option = 1;
  ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  INFO("setsockopt ret : %d\n", ret);
  ret = bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if (ret < 0) {
      ERROR("ERROR when binding the listen fd\n");
      Log(LOG_LEVEL_ERROR, "[SERVER] ERROR when binding the listen fd");
      return -1;
  }
  INFO("Server is bind on %s:%d\n", address, port);
  Log(LOG_LEVEL_INFO, "[SERVER] Server is bind on %s:%d", address, port);

  ret = listen(listen_fd, max_client);
  if (ret < 0) {
      ERROR("ERROR when listen function");
      Log(LOG_LEVEL_ERROR, "[SERVER] ERROR when listen function");
      return -1;
  }
  INFO("Server is now listening...\n");
  Log(LOG_LEVEL_INFO, "[SERVER] Server is now listening");

  return listen_fd;
}

/**
 * @brief Accepts a new incoming client connection.
 * 
 * This function accepts a connection from a client, logs the client's IP address, and returns the new client socket file descriptor.
 * 
 * @param listen_fd The file descriptor of the listening socket.
 * @param client_addr A pointer to a sockaddr_in structure to store the client's address information.
 * @param client_ip A buffer to store the client's IP address as a string.
 * @param max_client The maximum number of clients allowed.
 * @param nb_client The current number of connected clients.
 * 
 * @return The file descriptor of the new client socket, or -1 in case of an error.
 */
int accept_connection(int listen_fd, struct sockaddr_in* client_addr, char* client_ip, int max_client, int nb_client) {
    socklen_t addr_len = sizeof(struct sockaddr_in);
    
    if (nb_client >= max_client) {
        INFO("Maximum number of clients reached. Connection refused.\n");
        return -1;
    }
    
    int new_client_fd = accept(listen_fd, (struct sockaddr*)client_addr, &addr_len);
    if (new_client_fd < 0) {
        ERROR("ERROR when accept client");
        Log(LOG_LEVEL_ERROR, "[SERVER] ERROR when accept client");
        return -1;
    }

    INFO("New connection accepted: fd %d\n", new_client_fd);
    Log(LOG_LEVEL_INFO, "[SERVER] New client connection accepted: fd %d", new_client_fd);

    inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    INFO("Client IP: %s\n", client_ip);

    return new_client_fd;
}

/**
 * @brief Handles communication with a connected client.
 * 
 * This function manages the entire lifecycle of a client connection, reading the client's data, 
 * processing HTTP requests, and checking the validity of the request. It handles different protocols, 
 * and logs relevant connection events.
 * 
 * @param conn A pointer to a connection_t structure representing the client connection.
 * 
 * @return 0 on success, or 1 in case of an error.
 */
int handle_connection(connection_t* conn) {
  INFO("Handling connection...\n");

  ssize_t total_bytes_read = 0;
  conn->client_buffer_len = 0;

  while(1) {
    ssize_t bytes_read = read(conn->client_fd, conn->client_buffer + total_bytes_read, sizeof(conn->client_buffer) - total_bytes_read);

    if (bytes_read < 0) {
      ERROR("ERROR when reading client request");
      Log(LOG_LEVEL_ERROR, "[SERVER] ERROR when reading client request");
      return 1;
    }

    if (bytes_read == 0) {
      INFO("Client closed the connection.\n");
      Log(LOG_LEVEL_INFO, "[SERVER] Client %d closed the connection", conn->client_fd);
      return 1;
    }

    total_bytes_read += bytes_read;
    conn->client_buffer_len = total_bytes_read;
    if (conn->client_buffer_len < (ssize_t)sizeof(conn->client_buffer)) {
      conn->client_buffer[conn->client_buffer_len] = '\0';
    } else {
      conn->client_buffer[sizeof(conn->client_buffer) - 1] = '\0';
    }
    
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
        Log(LOG_LEVEL_WARN, "[SERVER] Client have write %d bytes and http havn't been recognize...", total_bytes_read);
      } else {
        Log(LOG_LEVEL_WARN, "[SERVER] This http request is to huge to handle.");
        WARN("Request too large to handle.\n");
      }
      return 1;
    }
  }
  return 0;
}

/**
 * @brief Processes an HTTP request from a client.
 * 
 * This function handles the logic for processing an HTTP request, including retrieving the host, checking 
 * if the host is allowed, resolving DNS if necessary, and connecting to the remote host. It also handles 
 * specific cases, such as requests to localhost or HTTPS format requests, and sends appropriate responses (e.g., 404, 403).
 * 
 * @param conn A pointer to a connection_t structure representing the client connection.
 * 
 * @return 0 on success, or 1 in case of an error.
 */
int handle_http(connection_t* conn) {
    INFO("Handle HTTP function\n");
    
    char host[256] = {0};
    if (get_http_host(conn->client_buffer, host, sizeof(host)) == 0) {
        INFO("Host: %s\n", host);
    } else {
        WARN("Failed to retrieve host from request.\n");
        Log(LOG_LEVEL_WARN, "[SERVER] %s made a request without a valid Host header.", conn->client_ip);
        return 1;
    }

    Log(LOG_LEVEL_INFO, "[SERVER] %s asked for %s", conn->client_ip, host);
    INFO("Checking if host '%s's is allowed ...\n", host);

    if (is_host_deny(host)) {
        WARN("%s is deny by the bocklist, Sending HTTP 403 Forbiden\n", host);
        write_on_socket_http_from_buffer(conn->client_fd, HTTP_403_RESPONSE, sizeof(HTTP_403_RESPONSE));
        return 1;
    };

    INFO("The host %s is allowed\n", host);
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
        WARN("client %s ask https format for %s\n, Sending a 404 not found", conn->client_ip, host);
        Log(LOG_LEVEL_WARN, "[SERVER] Client %s ask HTTPS format, Sending 404 not found", conn->client_ip);
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
            Log(LOG_LEVEL_ERROR, "[SERVER] Invalid IP address");
            free(ip);
            free(port);
            return 1;
        }

        // Creating the server socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            ERROR("server socket creation failed");
            Log(LOG_LEVEL_ERROR, "[SERVER] server socket creation failed");
            write_on_socket_http_from_buffer(conn->client_fd, HTTP_404_RESPONSE, sizeof(HTTP_404_RESPONSE));
            free(ip);
            free(port);
            return 3;
        }

        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
            ERROR("Failed to connect");
            Log(LOG_LEVEL_ERROR, "[SERVER] Failed to connect to %s", ip);
            write_on_socket_http_from_buffer(conn->client_fd, HTTP_404_RESPONSE, sizeof(HTTP_404_RESPONSE));
            close(sockfd);
            free(ip);
            free(port);
            return 4;
        }

        INFO("Connected to %s on port %s\n", ip, port);
        Log(LOG_LEVEL_INFO, "[SERVER] Connected to %s on port %s", ip, port);
        free(ip);
        free(port);
    } else {
        // Else: DNS resolution and connection
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
        Log(LOG_LEVEL_INFO, "[SERVER] Connected to %s on port 80\n", ipstr);
        freeaddrinfo(res);
    }

    // Writing the client's buffer on socker
    if (write_on_socket_http_from_buffer(sockfd, conn->client_buffer, conn->client_buffer_len) != 0) {
        ERROR("Error while writing on the socket to the host %s, IP %s", host, conn->server_ip);
        Log(LOG_LEVEL_ERROR, "[SERVER] Error while writing on the socket to the host %s, IP %s", host, conn->server_ip);

        close(sockfd);
        return 1;
    } else {
        INFO("Writing OK\n");
        Log(LOG_LEVEL_INFO, "[SERVER] Client %d have write %d bytes to server", conn->client_fd, conn->client_buffer_len);
    }

    conn->server_fd = sockfd;

    INFO("End of handle_http\n");
    return 0;
}
