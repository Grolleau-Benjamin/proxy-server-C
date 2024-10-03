#include "includes/utils.h"
#include "includes/server.h"

#define PORT 8080
#define ADDRESS "127.0.0.1"
#define MAX_CLIENT 10

int main() {
  int listen_fd;
  struct sockaddr_in client_addr;

  listen_fd = init_listen_socket(ADDRESS, PORT, MAX_CLIENT);

  struct pollfd fds[MAX_CLIENT + 1];
  connection_t *connections[MAX_CLIENT + 1];
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
          WARN("Error on socket %d, closing the connection\n", fds[i].fd);
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
        int new_client_fd = accept_connection(listen_fd, &client_addr);
        
        fds[nfds].fd = new_client_fd;
        fds[nfds].events = POLLIN;

        connections[nfds] = malloc(sizeof(connection_t));
        connections[nfds]->client_fd = new_client_fd;
        connections[nfds]->server_fd = -1;
        connections[nfds]->client_buffer_len = 0;
        connections[nfds]->server_buffer_len = 0;

        nfds++;
      } else {
        
        // TODO: Handle connection 
        
        INFO("Closing connection for fd %d\n", fds[i].fd);
        close(fds[i].fd);
        free(connections[i]);
        connections[i] = NULL;
        // If the connection is not the last in the list, replace it with the last one
        // to maintain a contiguous list and avoid gaps        fds[i].fd = -1;
        if (i < nfds - 1) {
          fds[i] = fds[nfds - 1];
          connections[i] = connections[nfds - 1];
          connections[nfds - 1] = NULL;
        }
        nfds --;
        INFO("Connection closed!\n");
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
  return 0;
}
