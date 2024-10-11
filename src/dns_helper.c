// dns_helper.c

#include "../includes/dns_helper.h"
#include "../includes/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static dns_cache_t *dns_cache = NULL;

int init_dns_cache() {
    if (dns_cache != NULL) {
        INFO("DNS cache is already initialized.\n");
        return 0;
    }

    dns_cache = malloc(sizeof(dns_cache_t));
    if (!dns_cache) {
        ERROR("Failed to allocate memory for DNS cache.\n");
        return -1;
    }
    dns_cache->head = NULL;
    INFO("DNS cache initialized.\n");
    return 0;
}

int add_in_cache(const char* host, const char* ipstr, struct addrinfo* addr_info) {
    if (!dns_cache || !host || !ipstr || !addr_info) {
        ERROR("Invalid arguments for add_in_cache.\n");
        return -1;
    }

    if (find_in_cache(host) != NULL) {
        INFO("Entry for %s already exists in DNS cache.\n", host);
        return 0;
    }

    dns_cache_entry_t *new_entry = malloc(sizeof(dns_cache_entry_t));
    if (!new_entry) {
        ERROR("Failed to allocate memory for DNS cache entry.\n");
        return -1;
    }

    strncpy(new_entry->host, host, sizeof(new_entry->host) - 1);
    new_entry->host[sizeof(new_entry->host) - 1] = '\0';
    strncpy(new_entry->ipstr, ipstr, sizeof(new_entry->ipstr) - 1);
    new_entry->ipstr[sizeof(new_entry->ipstr) - 1] = '\0';

    new_entry->addr_info = malloc(sizeof(struct addrinfo));
    if (!new_entry->addr_info) {
        ERROR("Failed to allocate memory for addrinfo.\n");
        free(new_entry);
        return -1;
    }
    memcpy(new_entry->addr_info, addr_info, sizeof(struct addrinfo));

    new_entry->next = dns_cache->head;
    dns_cache->head = new_entry;

    INFO("Added DNS cache entry: %s -> %s\n", host, ipstr);
    return 0;
}

dns_cache_entry_t* find_in_cache(const char* host) {
    if (!dns_cache || !host) {
        ERROR("Invalid arguments for find_in_cache.\n");
        return NULL;
    }

    dns_cache_entry_t *current = dns_cache->head;
    while (current) {
        if (strcmp(current->host, host) == 0) {
            INFO("Found DNS cache entry for host: %s -> %s\n", current->host, current->ipstr);
            return current;
        }
        current = current->next;
    }

    INFO("No DNS cache entry found for host: %s\n", host);
    return NULL;
}

void free_dns_cache() {
    if (!dns_cache) {
        INFO("DNS cache is not initialized or has already been freed.\n");
        return;
    }

    dns_cache_entry_t *current = dns_cache->head;
    while (current) {
        dns_cache_entry_t *next = current->next;
        if (current->addr_info) {
            freeaddrinfo(current->addr_info);
            free(current->addr_info);
        }
        free(current);
        current = next;
    }
    free(dns_cache);
    dns_cache = NULL;
    INFO("DNS cache freed.\n");
}

int resolve_dns(const char* host, struct addrinfo** res, char* ipstr) {
    if (!dns_cache) {
        ERROR("DNS cache is not initialized. Call init_dns_cache() first.\n");
        return -1;
    }

    INFO("Resolving DNS for %s\n", host);

    dns_cache_entry_t *cached_entry = find_in_cache(host);
    if (cached_entry) {
        strncpy(ipstr, cached_entry->ipstr, INET6_ADDRSTRLEN - 1);
        ipstr[INET6_ADDRSTRLEN - 1] = '\0';
        *res = cached_entry->addr_info;
        return 0;
    }

    struct addrinfo hints;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(host, "80", &hints, res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    struct sockaddr_in* ipv4 = (struct sockaddr_in*)(*res)->ai_addr;
    void* addr = &(ipv4->sin_addr);

    if (inet_ntop((*res)->ai_family, addr, ipstr, INET6_ADDRSTRLEN) == NULL) {
        perror("inet_ntop");
        freeaddrinfo(*res);
        return 3;
    }

    INFO("DNS resolution successful for %s -> %s\n", host, ipstr);

    if (add_in_cache(host, ipstr, *res) != 0) {
        ERROR("Failed to add DNS resolution to cache.\n");
    }

    return 0;
}
