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
 * @file http_helper.c
 * @brief Implementation of HTTP helper functions.
 *
 * This file contains the implementation of functions that assist in
 * handling and validating HTTP requests.
 */

#include <string.h>

#include "../includes/utils.h"
#include "../includes/http_helper.h"
#include "../includes/logger.h"



/**
 * @brief HTTP methods
 *
 * This list contains all HTTP methods
 */
const char* http_methods[] = {
  "GET", "POST", "HEAD", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"
};

/**
 * @brief Checks if the buffer contains a valid HTTP method.
 *
 * This function compares the start of the buffer with known HTTP methods
 * and returns 1 if a valid method is found, otherwise returns 0.
 *
 * @param buffer The buffer to check for an HTTP method.
 * @return 1 if the buffer contains a valid HTTP method, 0 otherwise.
 */
int is_http_method(const char* buffer) {
  for(size_t i = 0; i < sizeof(http_methods) / sizeof(http_methods[0]); i++) {
    if (strncmp(buffer, http_methods[i], strlen(http_methods[i])) == 0) {
      return 1;
    }
  }
  return 0;
}

/**
 * @brief Checks if an HTTP request is complete.
 *
 * This function checks if the buffer contains a complete HTTP request,
 * including the method, path, HTTP version, and necessary headers.
 *
 * @param buffer The buffer containing the HTTP request.
 * @return 1 if the request is complete, 0 otherwise.
 */
int is_http_request_complete(const char* buffer) {
    if (!buffer) return 0;
    
    if (strstr(buffer, "\r\n\r\n") == NULL) return 0;
    
    const char* first_line_end = strstr(buffer, "\r\n");
    if (!first_line_end) return 0;
    
    size_t first_line_length = first_line_end - buffer;
    if (first_line_length >= 256) return 0;
    
    char first_line[256];
    strncpy(first_line, buffer, first_line_length);
    first_line[first_line_length] = '\0';
    for (size_t i = 0; i < sizeof(http_methods) / sizeof(http_methods[0]); i++) {
        size_t method_len = strlen(http_methods[i]);
        if (strncmp(first_line, http_methods[i], method_len) == 0) {
            const char* path_start = first_line + method_len;
            if (*path_start == ' ') {
                const char* version_start = strstr(path_start + 1, "HTTP/1.1");
                if (version_start && strcmp(version_start, "HTTP/1.1") == 0) {
                    if (strstr(buffer, "Host: ") != NULL) {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

/**
 * @brief Extracts the host from an HTTP request.
 *
 * This function searches for the "Host" header in the HTTP request and
 * extracts the host value into the provided `host` buffer.
 *
 * @param buffer The buffer containing the HTTP request.
 * @param host The buffer to store the extracted host.
 * @param host_size The size of the `host` buffer.
 * @return 0 if the host was successfully extracted, -1 otherwise.
 */
int get_http_host(const char* buffer, char* host, size_t host_size) { 
  char* host_header = NULL;
  char* end_of_host = NULL;

  host_header = strstr(buffer, "Host: ");
  if (host_header) {
    host_header += 6; // Len of "Host: "

    end_of_host = strchr(host_header, '\r');
    if (!end_of_host) { end_of_host = strchr(host_header, '\n'); }

    if (end_of_host) {
      size_t host_length = end_of_host - host_header;
      if (host_length < host_size) {
        strncpy(host, host_header, host_length);
        host[host_length] = '\0';
        return 0;
      } else {
        WARN("Hostname too long\n");
        Log(LOG_LEVEL_WARN, "[SERVER] Hostname too long");
        return -1;
      }
    } else {
      WARN("End of Host header not found\n");
      return -1;
    }
  } else {
    WARN("Host not found in request\n"); 
    return -1;
  }
}
