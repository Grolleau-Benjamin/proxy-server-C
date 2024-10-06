#include "includes/config.h"
#include "includes/utils.h"
#include "includes/server.h"
#include "includes/logger.h"
#include "includes/rules.h"
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#define CONFIG_FILENAME "proxy.config"

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

  Log(LOG_LEVEL_INFO, "Application start.");
  
  int listen_fd;
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

      // If there is no events detected, juste skip the loop and go to next i
      if (fds[i].revents == 0) continue;

      // Errors on descriptor
      // Refer to: man poll
      if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)){
        if (i != 0) { // not the listening socket
          ERROR("Error on socket %d, closing the connection\n", fds[i].fd);
          close(fds[i].fd);
          if (connections[i]) {
            if (connections[i]->client_fd != -1) close(connections[i]->client_fd);
            if (connections[i]->server_fd != -1) close(connections[i]->server_fd);
            free(connections[i]);
            connections[i] = NULL;
          }
          fds[i].fd = -1;
        }
        continue;
      }

      // Action on the listenning socket => new connection
      if (fds[i].fd == listen_fd) {
        char client_ip[INET_ADDRSTRLEN];
        int new_client_fd = accept_connection(listen_fd, &client_addr, client_ip);
        
        fds[nfds].fd = new_client_fd;
        fds[nfds].events = POLLIN;

        connections[nfds] = malloc(sizeof(connection_t));
        if (connections[nfds] == NULL) {
          perror("malloc");
          close(new_client_fd);
        }
        
        connections[nfds]->client_fd = new_client_fd;
        connections[nfds]->server_fd = -1;
        connections[nfds]->client_buffer_len = 0;
        connections[nfds]->server_buffer_len = 0;
        strcpy(connections[nfds]->client_ip, client_ip);
        memset(connections[nfds]->server_ip, 0, sizeof(connections[nfds]->server_ip));

        nfds++;
      } else {
        int close_conn = handle_connection(connections[i]);
        
        if (close_conn) {
          INFO("Closing connection for fd %d\n", fds[i].fd);
          close(fds[i].fd);
          fds[i].fd = -1;
          fds[i].revents = 0;
          free(connections[i]);
          connections[i] = NULL;
          nfds --;
          INFO("Connection closed!\n");
        }
      }
    }

    // cleaning closed descriptors after each iteration to handle bug
    int new_nfds = 0;
    for(int i =0; i < nfds; i++) {
      if (fds[i].fd != -1) {
        if (i != new_nfds) {
          fds[new_nfds] = fds[i];
        }
        new_nfds ++;
      }
    }
    nfds = new_nfds;
  }
  
  close(listen_fd);
  close_logger();
  return EXIT_SUCCESS;
}
