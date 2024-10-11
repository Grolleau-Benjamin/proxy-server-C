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
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#define CONFIG_FILENAME "conf/proxy.config"


int main() {
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
  
  if (init_regex() != 0) {
    return EXIT_FAILURE;
  } else {
    Log(LOG_LEVEL_INFO, "Regex have been init.");
  }

  Log(LOG_LEVEL_INFO, "Application start.");
  
  struct sockaddr_in client_addr;
  int listen_fd = init_listen_socket(config.address, config.port, config.max_client);

  struct pollfd fds[config.max_client * 2 + 1];
  connection_t *connections[config.max_client * 2 + 1];
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
      // If there is no events detected, juste skip the loop and go to next i
      if (fds[i].revents == 0) continue;
      INFO("Activity on fd : %d\n", fds[i].fd);

      // Errors on descriptor
      // Refer to: man poll
      if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)){
        if (i != 0) { // not the listening socket
          WARN("Error (POLLERR | POLLHUP | POLLNVAL) on socket %d, closing the connection, error\n", fds[i].fd);
          close(fds[i].fd);
          if (connections[i]) {
            if (connections[i]->client_fd != -1) {
              close(connections[i]->client_fd);
              if (fds[i].fd == connections[i]->client_fd) {
                INFO("Error on client\n");
                INFO("Socket server fd: %d\n", connections[i]->server_fd);
                INFO("fds - 1 (%d): %d\n", i-1, fds[i-1].fd);
                INFO("fds (%d): %d\n", i, fds[i].fd);
                INFO("fds + 1 (%d): %d\n", i+1, fds[i+1].fd);
              }
            }
            if (connections[i]->server_fd != -1){
              close(connections[i]->server_fd);
              if (fds[i].fd == connections[i]->server_fd) {
                INFO("Error on server\n");
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
        memset(connections[nfds]->client_buffer, 0, sizeof(connections[nfds]->client_buffer));
        int close_conn = handle_connection(connections[nfds]);

        if (close_conn) {
          WARN("Clossing connections for %d\n", connections[nfds]->client_fd);
          close(connections[nfds]->client_fd);
          close(connections[nfds]->server_fd);
          fds[nfds].fd = -1;
          fds[nfds].revents = 0;
          free(connections[nfds]);
          continue;
        }

        INFO("Adding the server fd %d to poll\n", connections[nfds]->server_fd);
        nfds++; // Server fd position
        INFO("1 - nfds after adding: %d\n", nfds);
        INFO("Max nfds: %d\n", config.max_client);

        connections[nfds] = connections[nfds - 1];
        INFO("2 - nfds after adding: %d\n", nfds);
        fds[nfds].fd = connections[nfds]->server_fd;
        INFO("3 - nfds after adding: %d\n", nfds);
        fds[nfds].events = POLLIN;
        INFO("4 - nfds after adding: %d\n", nfds);
        fds[nfds].revents = 0;
        INFO("5 - nfds after adding: %d\n", nfds);
        INFO("fds[%d].fd = %d (server)\n", nfds, connections[nfds]->server_fd);
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
          INFO("Activity on client %d\n", fds[i].fd);
          // Lecture depuis le client et envoi au serveur
          ssize_t bytes = read(conn->client_fd, conn->client_buffer, sizeof(conn->client_buffer));
          if (bytes <= 0) {
            // Fermeture de la connexion
            // TODO Gerer mieux la fermeture et la deconnection
            close(conn->client_fd);
            close(conn->server_fd);
            // connections[i] = NULL;
            // Retirer les descripteurs du tableau
            fds[i].fd = -1;
            fds[i].revents =0;
            fds[i+1].fd = -1;
            fds[i+1].revents = 0;
            continue;
          }
            // Envoyer au serveur
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
          // Lecture depuis le serveur et envoi au client
          ssize_t bytes = read(conn->server_fd, conn->server_buffer, sizeof(conn->server_buffer));
          if (bytes <= 0) {
              INFO("Closing connection on server (%d), no more bits to read\n", conn->server_fd);
              // Fermeture de la connexion
              close(conn->client_fd);                           // TODO: 100% l'erreur est la
              // voir pour close ailleur ou autrement 
              INFO("Closed the connection on client: %d\n", conn->client_fd);
              close(conn->server_fd);
              INFO("Closed the connection on server: %d\n", conn->server_fd);

              INFO("On server fds\n");
              INFO("fds[%d] (i - 1) = %d\n", i-1, fds[i-1].fd);
              INFO("fds[%d] (i) = %d\n", i, fds[i].fd);
              
              // Retirer les descripteurs du tableau
              fds[i].fd = -1;
              fds[i].revents = 0;
              fds[i-1].fd = -1;
              fds[i-1].revents = 0;

              INFO("Connection close\n");
              continue;
          }
          // Envoyer au client
          int ret = write(conn->client_fd, conn->server_buffer, bytes);
          if (ret < 0) {
            perror("write to client");
            close(conn->client_fd);
            close(conn->server_fd);
            // connections[i] = NULL;
            fds[i].fd = -1;
            fds[i].revents =0;
            fds[i-1].fd = -1;
            fds[i-1].revents = 0;
            continue;
          } else {
            INFO("Write %d bytes to client %d from %d\n", ret, conn->client_fd, conn->server_fd);
            INFO("Buffer: %s\n", conn->server_buffer);
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
  return EXIT_SUCCESS;
}
