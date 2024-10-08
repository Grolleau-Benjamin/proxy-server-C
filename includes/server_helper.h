#ifndef SERVER_HELPER_H
#define SERVER_HELPER_H

int init_regex();
void free_regex();
int is_ip_port_format(const char *host, char **ip, char **port);
int is_host_https_format(const char* host);
int replace_localhost_with_ip(char* host);

#endif
