#include "../includes/server_helper.h"
#include "../includes/utils.h"

#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static regex_t ip_port_regex;
static regex_t https_regex;
static int regex_compiled = 0;

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

void free_regex() {
  if (regex_compiled) {
    regfree(&ip_port_regex);
    regfree(&https_regex);
    regex_compiled = 0;
  }
}

int is_ip_port_format(const char *host, char **ip, char **port) {
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

