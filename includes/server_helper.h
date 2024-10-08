#ifndef SERVER_HELPER_H
#define SERVER_HELPER_H

int init_regex();
void free_regex();
int is_ip_port_format(const char *host, char **ip, char **port);
int is_host_https_format(const char* host);
int replace_localhost_with_ip(char* host);
int write_on_socket_http_from_buffer(int fd, char* buffer, int buffer_len);
int read_on_socket_http(int fd, char* buffer, int buffer_size);

#endif
