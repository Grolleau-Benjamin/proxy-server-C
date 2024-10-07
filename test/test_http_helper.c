#include "../includes/http_helper.h"
#include "../includes/utils.h"

void check_result(int result, const char* description) {
    if (result) {
        INFO("PASS: %s\n", description);
    } else {
        ERROR("FAIL: %s\n", description);
    }
}

void test_is_http_method() {
    INFO("Testing is_http_method...\n");

    const char* valid_method = "GET";
    const char* invalid_method = "INVALID";

    check_result(is_http_method(valid_method), "Valid method is recognized as valid");
    check_result(!is_http_method(invalid_method), "Invalid method is correctly not recognized");
}

void test_is_http_request_complete() {
    INFO("Testing is_http_request_complete...\n");

    const char* complete_request = "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
    const char* incomplete_request = "GET / HTTP/1.1\r\nHost: example.com\r\n";
    const char* incomplete_request2 = "GET / HTTP/1.1\r\n\r\n";
    const char* incomplete_request3 = "GET / \r\nHost: example.com\r\n";
    const char* incomplete_request4 = "GET HTTP/1.1 \r\nHost: example.com\r\n";

    check_result(is_http_request_complete(complete_request), "Complete request is recognized as complete");
    check_result(!is_http_request_complete(incomplete_request), "Incomplete request without final CRLF is correctly not recognized");
    check_result(!is_http_request_complete(incomplete_request2), "Incomplete request without Host header is correctly not recognized");
    check_result(!is_http_request_complete(incomplete_request3), "Incomplete request without proper HTTP version is correctly not recognized");
    check_result(!is_http_request_complete(incomplete_request4), "Incomplete request with malformed request line is correctly not recognized");
}

void test_get_http_host() {
    INFO("Testing get_http_host...\n");

    const char* request = "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
    char host[256];
    int result = get_http_host(request, host, sizeof(host));
    check_result(result == 0 && strcmp(host, "example.com") == 0, "Host extracted successfully");

    const char* bad_request = "GET / HTTP/1.1\r\n\r\n"; 
    result = get_http_host(bad_request, host, sizeof(host));
    check_result(result == -1, "No host found as expected");
}

int main() {
    test_is_http_method();
    test_is_http_request_complete();
    test_get_http_host();
    return 0;
}
