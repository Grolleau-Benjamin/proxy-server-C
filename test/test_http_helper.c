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
#include <string.h>
#include "../includes/http_helper.h"
#include "../includes/utils.h"

void test_is_http_method() {
    INFO("Testing is_http_method...\n");

    const char* valid_method = "GET";
    const char* invalid_method = "INVALID";

    assert(is_http_method(valid_method));
    INFO("\tsuccess: Valid method has been recognized as valid\n");

    assert(!is_http_method(invalid_method));
    INFO("\tsuccess: Invalid method has been recognized as invalid\n");
}

void test_is_http_request_complete() {
    INFO("Testing is_http_request_complete...\n");

    const char* complete_request = "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
    const char* incomplete_request = "GET / HTTP/1.1\r\nHost: example.com\r\n";
    const char* incomplete_request2 = "GET / HTTP/1.1\r\n\r\n";
    const char* incomplete_request3 = "GET / \r\nHost: example.com\r\n";
    const char* incomplete_request4 = "GET HTTP/1.1 \r\nHost: example.com\r\n";

    assert(is_http_request_complete(complete_request));
    INFO("\tsuccess: Complete request has been recognized as complete\n");

    assert(!is_http_request_complete(incomplete_request));
    INFO("\tsuccess: Incomplete request without final CRLF has been recognized as incomplete\n");

    assert(!is_http_request_complete(incomplete_request2));
    INFO("\tsuccess: Incomplete request without Host header has been recognized as incomplete\n");

    assert(!is_http_request_complete(incomplete_request3));
    INFO("\tsuccess: Incomplete request without proper HTTP version has been recognized as incomplete\n");

    assert(!is_http_request_complete(incomplete_request4));
    INFO("\tsuccess: Incomplete request with malformed request line has been recognized as incomplete\n");
}

void test_get_http_host() {
    INFO("Testing get_http_host...\n");

    const char* request = "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
    char host[256];
    int result = get_http_host(request, host, sizeof(host));
    assert(result == 0 && strcmp(host, "example.com") == 0);
    INFO("\tsuccess: Host has been extracted successfully\n");

    const char* bad_request = "GET / HTTP/1.1\r\n\r\n"; 
    result = get_http_host(bad_request, host, sizeof(host));
    assert(result == -1);
    INFO("\tsuccess: No host found as expected\n");
}

int main() {
    test_is_http_method();
    test_is_http_request_complete();
    test_get_http_host();
    return 0;
}
