#include "../includes/server.h"
#include "../includes/utils.h"
#include <arpa/inet.h>

void check_result(int result, const char* description) {
    if (result) {
        INFO("PASS: %s\n", description);
    } else {
        ERROR("FAIL: %s\n", description);
    }
}

void test_init_listen_socket() {
  INFO("Testing init_listen_socket...\n");
  int port = 8080;
  int max_client = 5;
  const char* address = "127.0.0.1";

  int listen_fd = init_listen_socket(address, port, max_client);
  check_result(listen_fd > 0, "Listen socket is successfully created");

  close(listen_fd);
}

void test_handle_http() {
  INFO("Testing handle_http...\n");

  connection_t conn;
  memset(&conn, 0, sizeof(connection_t));

  conn.client_fd = 1;  
  snprintf(conn.client_ip, sizeof(conn.client_ip), "127.0.0.1");

  // Test with a valid HTTP request
  const char* valid_request = "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
  strcpy(conn.client_buffer, valid_request);
  conn.client_buffer_len = strlen(valid_request);

  check_result(handle_http(&conn) == 0, "Valid HTTP request is handled correctly");

  // Test with a request without Host
  const char* invalid_request = "GET / HTTP/1.1\r\n\r\n";
  strcpy(conn.client_buffer, invalid_request);
  conn.client_buffer_len = strlen(invalid_request);

  check_result(handle_http(&conn) == 1, "HTTP request without Host is rejected");
}

int main() {
  INFO("Running server.c tests...\n");

  test_init_listen_socket();
  test_handle_http();

  return 0;
}
