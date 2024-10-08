#ifndef HTTP_HELPER
#define HTTP_HELPER

#include <stdlib.h>

/**
 * @file http_helper.h
 * @brief Helper functions for handling HTTP requests.
 *
 * This file contains declarations for functions that help with the
 * parsing and validation of HTTP requests.
 */

int is_http_method(const char* buffer);
int is_http_request_complete(const char* buffer);
int get_http_host(const char* buffer, char* host, size_t host_size);

#endif
