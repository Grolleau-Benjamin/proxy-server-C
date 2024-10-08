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
#include "includes/logger.h"
#include "includes/rules.h"
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#define CONFIG_FILENAME "conf/proxy.config"

int listen_fd;

void handle_sigint(int sig) {
    INFO("Caught signal %d, shutting down...\n", sig);

    // Close the listen socket
    if (listen_fd != -1) {
        close(listen_fd);
        INFO("Closed listen_fd\n");
    }

    // Optional: Close logger and other resources
    close_logger();

    exit(0);  // Exit the program gracefully
}

int main() {
  signal(SIGINT, handle_sigint);

  INFO("Test info\n");     // TODO: Delete
  WARN("Test warn\n");     // TODO: Delete
  ERROR("Test error\n");   // TODO: Delete

  if (init_config(CONFIG_FILENAME) != 0) {
    return EXIT_FAILURE;
  } else {
    INFO("Config have been load correctly!\n");
  }
  
  if (init_logger(config.logger_filename) != 0) {
    return EXIT_FAILURE;
  } else {
    Log(LOG_LEVEL_INFO, "Logger have been correctly initialized!");
  }

  if (init_rules(config.rules_filename) != 0) {
    return EXIT_FAILURE;
  } else {
    Log(LOG_LEVEL_INFO, "Rules have been set.");
  }

  Log(LOG_LEVEL_INFO, "Application start.");
  

  struct sockaddr_in client_addr;

  listen_fd = init_listen_socket(config.address, config.port, config.max_client);

  struct pollfd fds[config.max_client + 1];
  connection_t *connections[config.max_client + 1];
  memset(fds, 0, sizeof(fds));
  memset(connections, 0, sizeof(connections));
  fds[0].fd = listen_fd;
  fds[0].events = POLLIN;
  INFO("Server's polls are ready!\n");

  int nfds = 1;

  while (1) {
    int activity = poll(fds, nfds, -1);
    INFO("Activity: %d\n", activity);
    print_error(activity, "poll");

    for (int i = 0; i < nfds; i++) {
      // INFO("fd : %d\n", fds[i].fd);
      // INFO("event : %d\n", fds[i].events);
      // INFO("revent : %d\n", fds[i].revents);
      // If there is no events detected, juste skip the loop and go to next i
      if (fds[i].revents == 0) continue;

      // Errors on descriptor
      // Refer to: man poll
      if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)){
        if (i != 0) { // not the listening socket
          ERROR("Error on socket %d, closing the connection, error\n", fds[i].fd);
          close(fds[i].fd);
          if (connections[i]) {
            if (connections[i]->client_fd != -1) close(connections[i]->client_fd);
            if (connections[i]->server_fd != -1) close(connections[i]->server_fd);
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
        int new_client_fd = accept_connection(listen_fd, &client_addr, client_ip);
        
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
        int close_conn = handle_connection(connections[nfds]);

        if (close_conn) {
          WARN("Clossing connections\n");
          close(connections[nfds]->client_fd);
          close(connections[nfds]->server_fd);
          fds[nfds].fd = -1;
          fds[nfds].events = 0;
          fds[nfds].revents = 0;
          free(connections[nfds]);
          continue;
        }
        INFO("Adding the server fd to poll\n");
        nfds++; // Server fd position

        connections[nfds] = connections[nfds -1];
        fds[nfds].fd = connections[nfds]->server_fd;
        fds[nfds].events = POLLIN;
        fds[nfds].revents = 0;
        INFO("Connected to the server\n");

        nfds++; // Next client fd position
      } else {
        //int close_conn = handle_connection(connections[i]);
        int ret;
        // verify if it's the server or the client or Null
        connection_t *conn = connections[i];

        if (conn == NULL)
          continue;

                        if (fds[i].fd == conn->client_fd && (fds[i].revents & POLLIN)) {
                    // Lecture depuis le client et envoi au serveur
                    ssize_t bytes = read(conn->client_fd, conn->client_buffer, sizeof(conn->client_buffer));
                    if (bytes <= 0) {
                        // Fermeture de la connexion
                        close(conn->client_fd);
                        close(conn->server_fd);
                        // connections[i] = NULL;
                        // Retirer les descripteurs du tableau
                        fds[i].fd = -1;
                        fds[i].revents =0;
                        continue;
                    }
                    // Envoyer au serveur
                    if (write(conn->server_fd, conn->client_buffer, bytes) < 0) {
                        perror("write to server");
                        close(conn->client_fd);
                        close(conn->server_fd);
                        // connections[i] = NULL;
                        fds[i].fd = -1;
                        fds[i].revents =0;
                        continue;
                    }
                }

                if (fds[i].fd == conn->server_fd && (fds[i].revents & POLLIN)) {
                    // Lecture depuis le serveur et envoi au client
                    ssize_t bytes = read(conn->server_fd, conn->server_buffer, sizeof(conn->server_buffer));
                    if (bytes <= 0) {
                        // Fermeture de la connexion
                        close(conn->client_fd);
                        close(conn->server_fd);
                        // connections[i] = NULL;
                        // Retirer les descripteurs du tableau
                        fds[i].fd = -1;
                        fds[i].revents =0;
                        continue;
                    }
                    // Envoyer au client
                    if (write(conn->client_fd, conn->server_buffer, bytes) < 0) {
                        perror("write to client");
                        close(conn->client_fd);
                        close(conn->server_fd);
                        // connections[i] = NULL;
                        fds[i].fd = -1;
                        fds[i].revents =0;
                        continue;
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
  }
  
  close(listen_fd);
  close_logger();
  free_rules();
  return EXIT_SUCCESS;
}
