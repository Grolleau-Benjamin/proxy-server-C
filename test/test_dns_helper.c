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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../includes/dns_helper.h"
#include "../includes/utils.h"

void test_init_dns_cache() {
    INFO("Testing init_dns_cache...\n");
    int result = init_dns_cache();
    assert(result == 0);
    INFO("\tsuccess: DNS cache initialized successfully\n");
}

void test_add_and_find_in_cache() {
    INFO("Testing add_in_cache and find_in_cache...\n");
    const char* host = "www.example.com";
    const char* ipstr = "93.184.216.34";

    struct addrinfo addr_info;
    memset(&addr_info, 0, sizeof(struct addrinfo));

    int add_result = add_in_cache(host, ipstr, &addr_info);
    assert(add_result == 0);
    INFO("\tsuccess: Entry added to DNS cache\n");

    dns_cache_entry_t* entry = find_in_cache(host);
    assert(entry != NULL);
    assert(strcmp(entry->host, host) == 0);
    assert(strcmp(entry->ipstr, ipstr) == 0);
    INFO("\tsuccess: Entry found in DNS cache\n");
}

void test_resolve_dns() {
    INFO("Testing resolve_dns...\n");
    const char* host = "www.google.com";
    char ipstr[INET6_ADDRSTRLEN];
    struct addrinfo* res = NULL;

    int resolve_result = resolve_dns(host, &res, ipstr);
    assert(resolve_result == 0);
    INFO("\tsuccess: DNS resolution successful\n");

    assert(strlen(ipstr) > 0);
    INFO("\tsuccess: IP address obtained: %s\n", ipstr);

    dns_cache_entry_t* entry = find_in_cache(host);
    assert(entry != NULL);
    INFO("\tsuccess: Entry added to DNS cache after resolution\n");

    if (res) {
        freeaddrinfo(res);
    }
}

void test_free_dns_cache() {
    INFO("Testing free_dns_cache...\n");
    free_dns_cache();
    INFO("\tsuccess: DNS cache freed successfully\n");
}

int main() {
    INFO("Running dns_helper.c tests...\n");

    test_init_dns_cache();
    test_add_and_find_in_cache();
    test_resolve_dns();
    test_free_dns_cache();

    INFO("All dns_helper.c tests passed successfully.\n");
    return 0;
}

