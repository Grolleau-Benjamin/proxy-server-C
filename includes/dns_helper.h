/**
 * @file dns_helper.h
 * @brief Header file for DNS helper functions and DNS cache management.
 */

#ifndef DNS_HELPER_H
#define DNS_HELPER_H

#include <arpa/inet.h>
#include <netdb.h>

/**
 * @struct dns_cache_entry
 * @brief Represents an entry in the DNS cache.
 */
typedef struct dns_cache_entry {
    char host[256];                    /**< The hostname. */
    char ipstr[INET6_ADDRSTRLEN];      /**< The IP address as a string. */
    struct addrinfo *addr_info;        /**< The deep-copied addrinfo structure. */
    struct dns_cache_entry *next;      /**< Pointer to the next cache entry. */
} dns_cache_entry_t;

/**
 * @struct dns_cache
 * @brief Represents the DNS cache.
 */
typedef struct dns_cache {
    dns_cache_entry_t *head;           /**< Pointer to the head of the cache entries list. */
} dns_cache_t;

int init_dns_cache();
dns_cache_entry_t* find_in_cache(const char* host);
int add_in_cache(const char* host, const char* ipstr, struct addrinfo* addr_info);
int resolve_dns(const char* host, struct addrinfo** res, char* ipstr);
void free_dns_cache();
struct addrinfo *copy_addrinfo(const struct addrinfo *src);

#endif 
