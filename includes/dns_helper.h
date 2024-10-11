// dns_helper.h

#ifndef DNS_HELPER_H
#define DNS_HELPER_H

#include <arpa/inet.h>
#include <netdb.h>

typedef struct dns_cache_entry {
    char host[256];                    
    char ipstr[INET6_ADDRSTRLEN];      
    struct addrinfo *addr_info;        
    struct dns_cache_entry *next;      
} dns_cache_entry_t;

typedef struct dns_cache {
    dns_cache_entry_t *head;           
} dns_cache_t;

int init_dns_cache();
dns_cache_entry_t* find_in_cache(const char* host);
int add_in_cache(const char* host, const char* ipstr, struct addrinfo* addr_info);
int resolve_dns(const char* host, struct addrinfo** res, char* ipstr);
void free_dns_cache();

#endif // DNS_HELPER_H
