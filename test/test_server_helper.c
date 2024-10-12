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
#include <stdlib.h>
#include <string.h>
#include "../includes/server_helper.h"
#include "../includes/utils.h"

void test_valid_ip_port(const char* input, const char* expected_ip, const char* expected_port) {
    INFO("Testing valid IP:port...\n");

    char* ip = NULL;
    char* port = NULL;

    int result = is_ip_port_format(input, &ip, &port);
    assert(result == 1);
    assert(strcmp(ip, expected_ip) == 0);
    assert(strcmp(port, expected_port) == 0);

    INFO("\tsuccess: IP and port correctly extracted\n");

    free(ip);
    free(port);

    INFO("\tTest passed for valid IP:port \"%s\"\n", input);
}

void test_invalid_ip_port(const char* input) {
    INFO("Testing invalid IP:port...\n");

    char* ip = NULL;
    char* port = NULL;

    int result = is_ip_port_format(input, &ip, &port);
    assert(result == 0);

    INFO("\tsuccess: Invalid IP:port correctly identified\n");
    INFO("\tTest passed for invalid IP:port \"%s\"\n", input);
}

void test_multiple_valid_cases() {
    INFO("Testing multiple valid cases...\n");

    test_valid_ip_port("192.168.1.1:80", "192.168.1.1", "80");
    test_valid_ip_port("255.255.255.254:65535", "255.255.255.254", "65535");
    test_valid_ip_port("0.0.0.1:1", "0.0.0.1", "1");
    test_valid_ip_port("192.168.1.1:8080", "192.168.1.1", "8080");

    INFO("\tsuccess: All valid cases passed\n");
}

void test_multiple_invalid_cases() {
    INFO("Testing multiple invalid cases...\n");

    test_invalid_ip_port("192.168.1.256:8080");   // IP outside range
    test_invalid_ip_port("192.168.1.1:-1");       // Negative port
    test_invalid_ip_port("192.168.1.1:0");        // Null port
    test_invalid_ip_port("192.168.1.1:70000");    // Port outside range
    test_invalid_ip_port("192.168.1.1");          // No port
    test_invalid_ip_port("abc.def.ghi.jkl:80");   // Non-numeric IP
    test_invalid_ip_port("123.456.78.90:1234");   // IP outside range
    test_invalid_ip_port("192.168.1.1:6553a");    // Invalid port

    INFO("\tsuccess: All invalid cases passed\n");
}

int main() {
    INFO("Running server_helper.c tests...\n");

    assert(init_regex() == 0);
    test_multiple_valid_cases();
    test_multiple_invalid_cases();
    free_regex();

    INFO("All tests passed!\n");

    return 0;
}
