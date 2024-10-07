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

#include "../includes/http_helper.h"

const char* http_methods[] = {
  "GET", "POST", "HEAD", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"
};

int is_http_method(const char* buffer) {
  for(size_t i = 0; i < sizeof(http_methods) / sizeof(http_methods[0]); i++) {
    if (strncmp(buffer, http_methods[i], strlen(http_methods[i])) == 0) {
      return 1;
    }
  }
  return 0;
}

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
        WARN("Host name too long\n");
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
