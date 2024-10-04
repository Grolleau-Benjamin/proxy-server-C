#ifndef HTTP_HELPER
#define HTTP_HELPER

#include <stdlib.h>
#include <string.h>

#include "utils.h"

int is_http_method(const char* buffer);
int is_http_request_complete(const char* buffer);
int get_http_host(const char* buffer, char* host, size_t host_size);

#endif
