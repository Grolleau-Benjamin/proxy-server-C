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
#include <arpa/inet.h>
#include "../includes/server.h"
#include "../includes/utils.h"

void test_init_listen_socket() {
  INFO("Testing init_listen_socket...\n");
  
  int port = 8080;
  int max_client = 5;
  const char* address = "127.0.0.1";

  int listen_fd = init_listen_socket(address, port, max_client);
  assert(listen_fd > 0);
  INFO("\tsuccess: Listen socket has been successfully created\n");

  close(listen_fd);
}

void test_handle_http() {
  INFO("Testing handle_http...\n");

  connection_t conn;
  memset(&conn, 0, sizeof(connection_t));

  conn.client_fd = 1;  
  snprintf(conn.client_ip, sizeof(conn.client_ip), "127.0.0.1");

  const char* valid_request = "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
  strcpy(conn.client_buffer, valid_request);
  conn.client_buffer_len = strlen(valid_request);

  assert(handle_http(&conn) == 0);
  INFO("\tsuccess: Valid HTTP request has been handled correctly\n");

  const char* invalid_request = "GET / HTTP/1.1\r\n\r\n";
  strcpy(conn.client_buffer, invalid_request);
  conn.client_buffer_len = strlen(invalid_request);

  assert(handle_http(&conn) == 1);
  INFO("\tsuccess: HTTP request without Host has been correctly rejected\n");
}

int main() {
  INFO("Running server.c tests...\n");

  test_init_listen_socket();
  test_handle_http();

  return 0;
}
