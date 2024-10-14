/**
 * @file server.h
 * @brief Header file for server-related operations, including socket management and connection handling.
 *
 * This header defines the structure and constants needed to manage client-server connections
 */
 
#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>

#include "http_helper.h"

#define BUFFER_SIZE 4096

/**
 * @brief Represents a connection between a client and a server.
 *
 * This structure stores all data for handling a connection, including file descriptors for
 * the client and server, buffers for storing client and server data, the length of data in those buffers,
 * and the IP addresses of both the client and server.
 */
typedef struct {
  int client_fd;                         /**< File descriptor for the client socket */
  int server_fd;                         /**< File descriptor for the server socket */
  char client_buffer[BUFFER_SIZE];       /**< Buffer for storing data received from the client */
  char server_buffer[BUFFER_SIZE];       /**< Buffer for storing data to send to the server */
  ssize_t client_buffer_len;             /**< Length of data in the client buffer */
  ssize_t server_buffer_len;             /**< Length of data in the server buffer */
  char client_ip[INET_ADDRSTRLEN];       /**< IP address of the client as a string */
  char server_ip[INET_ADDRSTRLEN];       /**< IP address of the server as a string */
} connection_t;

int init_listen_socket(const char* address, int port, int max_client);
int accept_connection(int listen_fd, struct sockaddr_in* client_addr, char* client_ip, int max_client, int nb_client);
int handle_connection(connection_t* conn);
int handle_http(connection_t* conn);

#endif
