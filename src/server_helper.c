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
 * @file server_helper.c
 * @brief Helper functions for server operations, including regex handling, socket reading/writing, and HTTP request processing.
 *
 * This file contains various utility functions to assist with server-side tasks such as
 * initializing and freeing regular expressions, identifying valid IP:Port or HTTPS formats, 
 * and handling read/write operations on sockets.
 */
 
#include "../includes/server_helper.h"
#include "../includes/utils.h"

#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Compiled regular expression for matching IP:Port format.
 * 
 * This regex is used to verify if a given host string follows the valid IP:Port format.
 */
static regex_t ip_port_regex;

/**
 * @brief Compiled regular expression for matching HTTPS host format.
 * 
 * This regex is used to verify if a given host string follows the HTTPS format (domain:443).
 */
static regex_t https_regex;

/**
 * @brief Flag to indicate whether the regular expressions are compiled.
 * 
 * This variable is used to ensure that the regular expressions for IP:Port 
 * and HTTPS format are compiled before they are used in matching operations.
 */
static int regex_compiled = 0;

/**
 * @brief Initializes the regular expressions for IP:Port and HTTPS format.
 * 
 * This function compiles two regular expressions, one for matching the IP:Port format
 * and another for matching the HTTPS format (domain:443). It sets the flag to indicate
 * that the regex patterns are ready to be used.
 * 
 * @return 0 on success, or 1 if an error occurs during regex compilation.
 */
int init_regex() {
  const char* ip_port_pattern = 
    "^((25[0-5]|"                                 // from 250 to 255
    "2[0-4][0-9]|"                                // from 200 to 249
    "1[0-9]{2}|"                                  // from 100 to 199
    "[1-9]?[0-9]"                                  // from 1 to 99
    ")\\.){3}"                                     // number + '.' 3 times
    "(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9]):" // another number with same rules
    "(6553[0-5]|"                                 // from 65530 to 65535
    "655[0-2][0-9]|"                              // from 65500 to 65529
    "65[0-4][0-9]{2}|"                            // from 65000 to 65499
    "6[0-4][0-9]{3}|"                              // from 60000 to 64999
    "[1-5][0-9]{4}|"                               // fron 10000 to 59999
    "[1-9][0-9]{0,3})$";                           // from 1 to 9999
  const char* https_pattern = "^[a-zA-Z0-9.-]+:443$";

  int ret = regcomp(&ip_port_regex, ip_port_pattern, REG_EXTENDED); 
  if (ret != 0) {
    ERROR("Echec during the compilation of the IP:Port regex\n");
    print_error(ret, "regex");
    return 1;
  }

  ret = regcomp(&https_regex, https_pattern, REG_EXTENDED); 
  if (ret != 0) {
    ERROR("Echec during the compilation of the HTTPS regex\n");
    print_error(ret, "regex");
    return 1;
  }

  regex_compiled = 1;
  return 0;
}

/**
 * @brief Frees the compiled regular expressions.
 * 
 * This function releases the memory associated with the compiled regular expressions
 * for IP:Port and HTTPS format, and resets the flag indicating that the regex is compiled.
 */
void free_regex() {
  if (regex_compiled) {
    regfree(&ip_port_regex);
    regfree(&https_regex);
    regex_compiled = 0;
  }
}

/**
 * @brief Checks if the given host matches the IP:Port format.
 * 
 * This function checks whether the provided host string follows the IP:Port format.
 * If it does, it extracts the IP and port values from the host string.
 * 
 * @param host The host string to check.
 * @param ip Pointer to store the extracted IP address.
 * @param port Pointer to store the extracted port number.
 * 
 * @return 1 if the host is in IP:Port format, 0 otherwise.
 */
int is_ip_port_format(const char *host, char **ip, char **port) {
  *ip = NULL;
  *port = NULL;

  if (!regex_compiled) {
    ERROR("Regex is not compiled. Call init_regex() before.\n");
    print_error(-1, "regex_not_compiled");
    return 0;
  }

  if (regexec(&ip_port_regex, host, 0, NULL, 0) == 0) {
    char* split_pos = strchr(host, ':');
    if (split_pos == NULL) return 0;
    size_t ip_len = split_pos - host;
    *ip = strndup(host, ip_len);
    *port = strdup(split_pos + 1); 
    return 1;
  }
  return 0;
}

/**
 * @brief Checks if the given host matches the HTTPS format.
 * 
 * This function checks whether the provided host string follows the HTTPS format (domain:443).
 * 
 * @param host The host string to check.
 * 
 * @return 1 if the host is in HTTPS format, 0 otherwise.
 */
int is_host_https_format(const char* host) {
  if (!regex_compiled) {
    ERROR("Regex is not compiled. Call init_regex() before.\n");
    print_error(-1, "regex_not_compiled");
    return 0;
  }

  if (regexec(&https_regex, host, 0, NULL, 0) == 0) {
    return 1;
  }
  return 0;
}

/**
 * @brief Replaces "localhost" with "127.0.0.1" in a host string.
 * 
 * This function searches for "localhost" in the provided host string and replaces it
 * with the IP address "127.0.0.1". It also preserves the port if specified in the string.
 * 
 * @param host The host string to modify.
 * 
 * @return 1 if the replacement was successful, 0 otherwise.
 */
int replace_localhost_with_ip(char* host) {
    const char* localhost = "localhost";
    const char* localhost_ip = "127.0.0.1";
    char* split_pos = strchr(host, ':');

    if (split_pos != NULL && strncmp(host, localhost, strlen(localhost)) == 0) {
        char* port = strdup(split_pos + 1);

        snprintf(host, strlen(localhost_ip) + strlen(port) + 2, "%s:%s", localhost_ip, port);

        free(port);
        return 1;
    }
    return 0;
}

/**
 * @brief Writes data from a buffer to a socket.
 * 
 * This function writes the content of the buffer to the specified socket file descriptor
 * until all data has been sent. It handles partial sends by looping until the buffer is fully written.
 * 
 * @param fd The file descriptor of the socket to write to.
 * @param buffer The buffer containing the data to send.
 * @param buffer_len The length of the buffer in bytes.
 * 
 * @return 0 on success, or 1 if an error occurs during writing.
 */
int write_on_socket_http_from_buffer(int fd, char* buffer, int buffer_len) {
    INFO("Writing on the fd %d ...\n", fd);
    int total_sent = 0;

    while (total_sent < buffer_len) {
        int temp_send = write(fd, buffer + total_sent, buffer_len - total_sent);
        INFO("write %d bytes\n", temp_send);
        if (temp_send == -1) {
            perror("write on server socket");
            return 1;
        }
        total_sent += temp_send;
    }
    INFO("Write done on fd %d\n", fd);
    return 0;
}

/**
 * @brief Reads data from a socket into a buffer.
 * 
 * This function reads data from the specified socket file descriptor into the provided buffer
 * until either the end of the HTTP headers ("\r\n\r\n") is detected or the buffer is full.
 * 
 * @param fd The file descriptor of the socket to read from.
 * @param buffer The buffer to store the received data.
 * @param buffer_size The size of the buffer in bytes.
 * 
 * @return The total number of bytes read, or -1 if an error occurs.
 */
int read_on_socket_http(int fd, char* buffer, int buffer_size) {
  INFO("Reading from the socket of fd %d\n", fd);
  int total_bytes_read = 0;
  int bytes_read;

  while (total_bytes_read != buffer_size) {
    bytes_read = read(fd, buffer + total_bytes_read, buffer_size - total_bytes_read);
    if (bytes_read == 0) return total_bytes_read;
    INFO("Bytes read = %d\n", bytes_read);
    
    total_bytes_read += bytes_read;
    if (strstr(buffer, "\r\n\r\n")) {
      buffer[total_bytes_read + 1] = '\0';
      INFO("Read done, total bytes : %d\n", total_bytes_read);
      return total_bytes_read;
    }
  }
  return total_bytes_read;
}

