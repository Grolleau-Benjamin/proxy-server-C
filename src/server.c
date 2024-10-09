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
      if (ret == 1) { return 1; }
      memset(conn->client_buffer, 0, sizeof(conn->client_buffer));
      conn->client_buffer_len = 0;
      total_bytes_read = 0;
      return 0;
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

    // Si le format IP:Port est valide, on se connecte directement
    if (is_host_https_format(host)) {
        WARN("https format for %s\n", host);
        WARN("Sending a 404 not found\n");
        write_on_socket_http_from_buffer(conn->client_fd, HTTP_404_RESPONSE, sizeof(HTTP_404_RESPONSE));
        return 1;
    }
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

        // Création du socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            ERROR("socket creation failed");
            return 3;
        }

        // Connexion directe
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
            ERROR("Failed to connect");
            close(sockfd);
            return 4;
        }

        INFO("Connected to %s on port %s\n", ip, port);
    } else {
        // Sinon, résolution DNS et connexion
        struct addrinfo hints;
        int status;
        char ipstr[INET6_ADDRSTRLEN];

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;  // Utilisation d'IPv4
        hints.ai_socktype = SOCK_STREAM;  // Sockets TCP

        // Récupération des infos d'adresse pour le hostname, port 80 (HTTP)
        if ((status = getaddrinfo(host, "80", &hints, &res)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
            return 2;
        }

        // Utilisation du premier résultat
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
        void *addr = &(ipv4->sin_addr);

        // Création du socket
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd == -1) {
            ERROR("socket creation failed");
            freeaddrinfo(res);
            return 3;
        }

        // Connexion au serveur
        if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
            ERROR("Failed to connect");
            close(sockfd);
            freeaddrinfo(res);
            return 4;
        }

        inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr));
        INFO("Connected to %s on port 80\n", ipstr);
    }

    // Ecriture dans le socket
    if (write_on_socket_http_from_buffer(sockfd, conn->client_buffer, conn->client_buffer_len) != 0) {
        ERROR("Error while writing on the socket to the host %s, IP %s", host, conn->server_ip);
        Log(LOG_LEVEL_ERROR, "Error while writing on the socket to the host %s, IP %s", host, conn->server_ip);

        if (res != NULL) {
            freeaddrinfo(res);
        }
        close(sockfd);
        return 1;
    } else {
        INFO("Writing OK\n");
    }

    // Assignation du socket au serveur
    conn->server_fd = sockfd;

    // Libération des ressources
    if (res != NULL) {
        freeaddrinfo(res);
    }

    INFO("End of handle_http\n");
    return 0;
}
