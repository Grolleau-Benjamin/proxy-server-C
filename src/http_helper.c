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
  const char* end_of_headers = "\r\n\r\n";
  if (strstr(buffer, end_of_headers) != NULL) {
    return 1;
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
        INFO("Host: %s\n", host);
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
