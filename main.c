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

/*
 * @file main.c
 * @brief Main entry point for the HTTP proxy server.
 *
 * This file contains the main function that initializes the server, handles incoming client connections,
 * and processes requests through polling. It also manages the lifecycle of the server, including setting
 * up configuration, logger, rules, and regular expressions. The server listens for client requests, forwards
 * them to the appropriate destination, and closes connections when necessary.
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
#include <unistd.h>
#include <signal.h>

#define CONFIG_FILENAME "conf/proxy.config"

// volatile because the value can change at any time (https://barrgroup.com/blog/how-use-cs-volatile-keyword)
volatile int running = 1;
void handle_signal(int signal) {
  running = 0;
}

int main() {

  
  INFO("Test info\n");     // TODO: Delete
  WARN("Test warn\n");     // TODO: Delete
  ERROR("Test error\n");   // TODO: Delete

  signal(SIGINT, handle_signal);

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
    Log(LOG_LEVEL_INFO, "[LOGGER] Logger have been correctly initialized");
  }

  if (init_rules(config.rules_filename) != 0) {
    ERROR("Loading rules failed...\n");
    close_logger();
    free_rules();
    return EXIT_FAILURE;
  } else {
    Log(LOG_LEVEL_INFO, "[CONFIG] Rules have been set.");
  }
  
  if (init_regex() != 0) {
    ERROR("Init regex failed...\n");
    close_logger();
    free_rules();
    free_regex();
    return EXIT_FAILURE;
  } else {
    Log(LOG_LEVEL_INFO, "[CONFIG] Regex have been init.");
  }

  if (init_dns_cache() != 0) {
    ERROR("Init DNS cache failed.\n");
    close_logger();
    free_rules();
    free_regex();
    return EXIT_FAILURE;
  } else {
    Log(LOG_LEVEL_INFO, "[CONFIG] DNS cache have been init.");
  }

  Log(LOG_LEVEL_INFO, "[SERVER] server starting....");
  
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
  Log(LOG_LEVEL_INFO, "[SERVER] Socket open on fd %d", listen_fd);

  struct pollfd fds[config.max_client * 2 + 1];
  connection_t *connections[config.max_client * 2 + 1];
  memset(fds, 0, sizeof(fds));
  memset(connections, 0, sizeof(connections));
  fds[0].fd = listen_fd;
  fds[0].events = POLLIN;
  INFO("Server's polls are ready!\n");
  Log(LOG_LEVEL_INFO, "[SERVER] Server polls are ready to run.");

  int nfds = 1;

  while (running) {
    int activity = poll(fds, nfds, -1);
    INFO("Activity: %d\n", activity);
    if (activity < 0) {
      if (errno == EINTR) {
        if (!running) {
          WARN("CTRL+C was pressed, the program is closing\n");
          INFO("Exiting main loop\n");
          break;
        } else {
          // Poll was interrupted but we're still running
          continue;
        }
      } else {
        ERROR("poll\n");
        break; // Handle other errors appropriately
      }
    }

    if (!running) {
      INFO("Exiting main loop\n");
      break;
    }

    for (int i = 0; i < nfds; i++) {
      // If there is no events detected, juste skip the loop and go to next i
      if (fds[i].revents == 0) continue;
      INFO("Activity on fd : %d\n", fds[i].fd);

      // Errors on descriptor
      // Refer to: man poll
      if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)){
        if (i != 0) { // not the listening socket
          ERROR("Error (POLLERR | POLLHUP | POLLNVAL) on socket %d, closing the connection, error\n", fds[i].fd);
          Log(LOG_LEVEL_ERROR, "[SERVER] Error (POLLERR | POLLHUP | POLLNVAL) on socket %d, closing the connection, error", fds[i].fd);
          close(fds[i].fd);
          if (connections[i]) {
            if (connections[i]->client_fd != -1) {
              close(connections[i]->client_fd);
              if (fds[i].fd == connections[i]->client_fd) {
                INFO("Error on client\n");
                INFO("Socket server fd: %d\n", connections[i]->server_fd);
                Log(LOG_LEVEL_ERROR, "[SERVER] Error on client, socker server fd: %d", connections[i]->server_fd);
              }
              connections[i]->client_fd = -1;
            }

            if (connections[i]->server_fd != -1){
              close(connections[i]->server_fd);
              if (fds[i].fd == connections[i]->server_fd) {
                INFO("Error on server\n");
                INFO("Socket client fd: %d\n", connections[i]->client_fd);
                Log(LOG_LEVEL_ERROR, "[SERVER] Error on server, socker client fd: %d", connections[i]->client_fd);
              }
              connections[i]->server_fd = -1;
            }
            // free(connections[i]);
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
        Log(LOG_LEVEL_INFO, "[SERVER] New client connected on socket %d", new_client_fd);
        
        fds[nfds].fd = new_client_fd;
        fds[nfds].events = POLLIN;

        connections[nfds] = malloc(sizeof(connection_t));
        if (connections[nfds] == NULL) {
          ERROR("malloc\n");
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
          Log(LOG_LEVEL_WARN, "[SERVER] Closing connections for %d", connections[nfds]->client_fd);
          if (connections[nfds]->client_fd != -1) close(connections[nfds]->client_fd);
          if (connections[nfds]->server_fd != -1) close(connections[nfds]->server_fd);
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
            Log(LOG_LEVEL_INFO, "[SERVER] Closing connection on client (%d), no more bits to read", conn->client_fd);
            close(conn->client_fd);
            close(conn->server_fd);
            fds[i].fd = -1;
            fds[i].revents =0;
            fds[i+1].fd = -1;
            fds[i+1].revents = 0;
            free(connections[i]);
            INFO("Connection close\n");
            continue;
          }
          if (write(conn->server_fd, conn->client_buffer, bytes) < 0) {
            ERROR("write to server\n");
            close(conn->client_fd);
            close(conn->server_fd);
            fds[i].fd = -1;
            fds[i].revents =0;
            fds[i+1].fd = -1;
            fds[i+1].revents = 0;
            free(connections[i]);
            continue;
          }
        }

        if (fds[i].fd == conn->server_fd && (fds[i].revents & POLLIN)) {
          INFO("Activity on server %d\n", fds[i].fd);
          ssize_t bytes = read(conn->server_fd, conn->server_buffer, sizeof(conn->server_buffer));
          if (bytes <= 0) {
              INFO("Closing connection on server (%d), no more bits to read\n", conn->server_fd);
              Log(LOG_LEVEL_INFO, "[SERVER] Closing connection on server (%d), no more bits to read", conn->server_fd);
              close(conn->client_fd);
              close(conn->server_fd);
              fds[i].fd = -1;
              fds[i].revents = 0;
              fds[i-1].fd = -1;
              fds[i-1].revents = 0;
              free(connections[i]);
              INFO("Connection close\n");
              continue;
          }
          int ret = write(conn->client_fd, conn->server_buffer, bytes);
          if (ret < 0) {
            ERROR("write to client\n");

            close(conn->client_fd);
            close(conn->server_fd);
            fds[i].fd = -1;
            fds[i].revents =0;
            fds[i-1].fd = -1;
            fds[i-1].revents = 0;
            free(connections[i]);
            continue;
          } else {
            INFO("Write %d bytes to client %d from %d\n", ret, conn->client_fd, conn->server_fd);
          }
        }
      }
    }

    // cleaning closed descriptors after each iteration to handle bug
    // Clean up closed descriptors and adjust the fds and connections arrays
    int new_nfds = 0;
    for (int i = 0; i < nfds; i++) {
      if (fds[i].fd != -1) {
        if (i != new_nfds) {
          fds[new_nfds] = fds[i];
          connections[new_nfds] = connections[i];
          connections[i] = NULL;
        }
        new_nfds++;
      }
    }
    nfds = new_nfds;
  }

  // Close all client and server connections
  for (int i = 1; i < config.max_client * 2 + 1 ; i += 2) {
      if (connections[i] != NULL) {
        if (connections[i]->client_fd != -1) {
          close(connections[i]->client_fd);
          connections[i]->client_fd = -1;
        }
        if (connections[i]->server_fd != -1) {
          close(connections[i]->server_fd);
          connections[i]->server_fd = -1;
        }
        free(connections[i]);
        connections[i] = NULL;
        connections[i+1] = NULL;
      }
  }

  close(listen_fd);
  close_logger();
  free_rules();
  free_regex();
  free_dns_cache();
  INFO("Shutdown complete.\n");
  return EXIT_SUCCESS;
}
