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

#include "includes/config.h"
#include "includes/utils.h"
#include "includes/server.h"
#include "includes/server_helper.h"
#include "includes/logger.h"
#include "includes/rules.h"
#include "includes/dns_helper.h"
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#define CONFIG_FILENAME "conf/proxy.config"

int main() {
  INFO("Test info\n");     // TODO: Delete
  WARN("Test warn\n");     // TODO: Delete
  ERROR("Test error\n");   // TODO: Delete

  if (init_config(CONFIG_FILENAME) != 0) {
    ERROR("Loading config file failed...\n");
    return EXIT_FAILURE;
  } else {
    INFO("Config have been load correctly!\n");
  }
  
  if (init_logger(config.logger_filename) != 0) {
    ERROR("Loading logger function failed...\n");
    close_logger();
    return EXIT_FAILURE;
  } else {
    Log(LOG_LEVEL_INFO, "Logger have been correctly initialized!");
  }

  if (init_rules(config.rules_filename) != 0) {
    ERROR("Loading rules failed...\n");
    close_logger();
    free_rules();
    return EXIT_FAILURE;
  } else {
    Log(LOG_LEVEL_INFO, "Rules have been set.");
  }
  
  if (init_regex() != 0) {
    ERROR("Init regex failed...\n");
    close_logger();
    free_rules();
    free_regex();
    return EXIT_FAILURE;
  } else {
    Log(LOG_LEVEL_INFO, "Regex have been init.");
  }

  if (init_dns_cache() != 0) {
    ERROR("Init DNS cache failed.\n");
    close_logger();
    free_rules();
    free_regex();
    return EXIT_FAILURE;
  }

  Log(LOG_LEVEL_INFO, "Application start.");
  
  struct sockaddr_in client_addr;
  int listen_fd = init_listen_socket(config.address, config.port, config.max_client);
  if (listen_fd < 0) {
    ERROR("Error while creating the proxy socket\n");
    close_logger();
    free_rules();
    free_regex();
    free_dns_cache();
    exit(EXIT_FAILURE);
  }
  Log(LOG_LEVEL_INFO, "Socket open on fd %d", listen_fd);

  struct pollfd fds[config.max_client * 2 + 1];
  connection_t *connections[config.max_client * 2 + 1];
  memset(fds, 0, sizeof(fds));
  memset(connections, 0, sizeof(connections));
  fds[0].fd = listen_fd;
  fds[0].events = POLLIN;
  INFO("Server's polls are ready!\n");
  Log(LOG_LEVEL_INFO, "Server polls are ready to run.");

  int nfds = 1;

  while (1) {
    int activity = poll(fds, nfds, -1);
    INFO("Activity: %d\n", activity);
    print_error(activity, "poll");

    for (int i = 0; i < nfds; i++) {
      // If there is no events detected, juste skip the loop and go to next i
      if (fds[i].revents == 0) continue;
      INFO("Activity on fd : %d\n", fds[i].fd);

      // Errors on descriptor
      // Refer to: man poll
      if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)){
        if (i != 0) { // not the listening socket
          ERROR("Error (POLLERR | POLLHUP | POLLNVAL) on socket %d, closing the connection, error\n", fds[i].fd);
          Log(LOG_LEVEL_ERROR, "Error (POLLERR | POLLHUP | POLLNVAL) on socket %d, closing the connection, error", fds[i].fd);
          close(fds[i].fd);
          if (connections[i]) {
            if (connections[i]->client_fd != -1) {
              close(connections[i]->client_fd);
              if (fds[i].fd == connections[i]->client_fd) {
                INFO("Error on client\n");
                INFO("Socket server fd: %d\n", connections[i]->server_fd);
                Log(LOG_LEVEL_ERROR, "Error on client, socker server fd: %d", connections[i]->server_fd);
              }
            }
            if (connections[i]->server_fd != -1){
              close(connections[i]->server_fd);
              if (fds[i].fd == connections[i]->server_fd) {
                INFO("Error on server\n");
                INFO("Socket client fd: %d\n", connections[i]->client_fd);
                Log(LOG_LEVEL_ERROR, "Error on server, socker client fd: %d", connections[i]->client_fd);
              }
              
            }
            free(connections[i]);
          }
          fds[i].fd = -1;
          fds[i].revents = 0;
        }
        continue;
      }

      // Action on the listenning socket => new connection
      if (fds[i].fd == listen_fd && (fds[i].revents & POLLIN) == POLLIN) {
        char client_ip[INET_ADDRSTRLEN];
        int new_client_fd = accept_connection(listen_fd, &client_addr, client_ip, config.max_client * 2, nfds);
        Log(LOG_LEVEL_INFO, "New client connected on socket %d", new_client_fd);
        
        fds[nfds].fd = new_client_fd;
        fds[nfds].events = POLLIN;

        connections[nfds] = malloc(sizeof(connection_t));
        if (connections[nfds] == NULL) {
          perror("malloc");
          close(new_client_fd);
          fds[nfds].fd = -1;
          fds[nfds].events = 0;
          fds[nfds].revents = 0;
        }
        
        connections[nfds]->client_fd = new_client_fd;
        connections[nfds]->server_fd = -1;
        connections[nfds]->client_buffer_len = 0;
        connections[nfds]->server_buffer_len = 0;
        strcpy(connections[nfds]->client_ip, client_ip);
        memset(connections[nfds]->server_ip, 0, sizeof(connections[nfds]->server_ip));
        memset(connections[nfds]->client_buffer, 0, sizeof(connections[nfds]->client_buffer));
        int close_conn = handle_connection(connections[nfds]);

        if (close_conn) {
          WARN("Clossing connections for %d\n", connections[nfds]->client_fd);
          Log(LOG_LEVEL_WARN, "Closing connections for %d", connections[nfds]->client_fd);
          close(connections[nfds]->client_fd);
          close(connections[nfds]->server_fd);
          fds[nfds].fd = -1;
          fds[nfds].revents = 0;
          free(connections[nfds]);
          continue;
        }

        INFO("Adding the server fd %d to poll\n", connections[nfds]->server_fd);
        nfds++; // Server fd position
        connections[nfds] = connections[nfds - 1];
        fds[nfds].fd = connections[nfds]->server_fd;
        fds[nfds].events = POLLIN;
        fds[nfds].revents = 0;
        INFO("Connected to the server\n");

        nfds++; // Next client fd position
      } else {
        int ret;
        // verify if it's the server or the client or Null
        connection_t *conn = connections[i];

        if (conn == NULL)
          continue;

        if (fds[i].fd == conn->client_fd && (fds[i].revents & POLLIN)) {
          INFO("Activity on client %d\n", fds[i].fd);
          ssize_t bytes = read(conn->client_fd, conn->client_buffer, sizeof(conn->client_buffer));
          if (bytes <= 0) {
            INFO("Closing connection on client (%d), no more bits to read\n", conn->client_fd);
            Log(LOG_LEVEL_INFO, "Closing connection on client (%d), no more bits to read", conn->client_fd);
            close(conn->client_fd);
            close(conn->server_fd);
            fds[i].fd = -1;
            fds[i].revents =0;
            fds[i+1].fd = -1;
            fds[i+1].revents = 0;
            INFO("Connection close\n");
            continue;
          }
          if (write(conn->server_fd, conn->client_buffer, bytes) < 0) {
            perror("write to server");
            close(conn->client_fd);
            close(conn->server_fd);
            fds[i].fd = -1;
            fds[i].revents =0;
            fds[i+1].fd = -1;
            fds[i+1].revents = 0;
            continue;
          }
        }

        if (fds[i].fd == conn->server_fd && (fds[i].revents & POLLIN)) {
          INFO("Activity on server %d\n", fds[i].fd);
          ssize_t bytes = read(conn->server_fd, conn->server_buffer, sizeof(conn->server_buffer));
          if (bytes <= 0) {
              INFO("Closing connection on server (%d), no more bits to read\n", conn->server_fd);
              Log(LOG_LEVEL_INFO, "Closing connection on server (%d), no more bits to read", conn->server_fd);
              close(conn->client_fd);
              close(conn->server_fd);
              fds[i].fd = -1;
              fds[i].revents = 0;
              fds[i-1].fd = -1;
              fds[i-1].revents = 0;

              INFO("Connection close\n");
              continue;
          }
          int ret = write(conn->client_fd, conn->server_buffer, bytes);
          if (ret < 0) {
            perror("write to client");
            close(conn->client_fd);
            close(conn->server_fd);
            fds[i].fd = -1;
            fds[i].revents =0;
            fds[i-1].fd = -1;
            fds[i-1].revents = 0;
            continue;
          } else {
            INFO("Write %d bytes to client %d from %d\n", ret, conn->client_fd, conn->server_fd);
            Log(LOG_LEVEL_INFO, "Write %d bytes to client %d from %d", ret, conn->client_fd, conn->server_fd);
          }
        }
      }
    }

    // cleaning closed descriptors after each iteration to handle bug
    int new_nfds = 0;
    for(int i =0; i < nfds; i++) {
      if (fds[i].fd != -1) {
        if (i != new_nfds) {
          fds[new_nfds] = fds[i];
          connections[new_nfds] = connections[i];
          connections[i] = NULL;
        }
        new_nfds ++;
      }
    }
    nfds = new_nfds;
    WARN("nfds: %d\n", nfds);
  }
  
  close(listen_fd);
  close_logger();
  free_rules();
  free_dns_cache();
  return EXIT_SUCCESS;
}
