#ifndef HTTP_HELPER
#define HTTP_HELPER

#include <stdlib.h>

#define HTTP_404_RESPONSE "HTTP/1.1 404 Not Found\r\n" \
                        "Content-Type: text/html\r\n" \
                        "Content-Length: 13\r\n" \
                        "\r\n" \
                        "<h1>404</h1>"

#define HTTP_401_RESPONSE "HTTP/1.1 401 Unauthorized\r\n"  \
                        "Date: Wed, 09 Oct 2024 15:34:48 GMT\r\n" \
                        "Server: Apache/2.4.41 (Ubuntu)\r\n" \
                        "WWW-Authenticate: Basic realm=\"Access to the site\"\r\n"  \
                        "Content-Type: text/html; charset=UTF-8\r\n" \
                        "Content-Length: 345\r\n" \
                        "Connection: close\r\n" \
                        "\r\n" \
                        "<html>\r\n" \
                        "<head>\r\n" \
                        "    <title>401 Unauthorized</title>\r\n" \
                        "</head>\r\n" \
                        "<body>\r\n" \
                        "    <h1>Unauthorized</h1>\r\n" \
                        "    <p>Authentication is required to access this resource. </p>\r\n" \
                        "</body>\r\n" \
                        "</html>\r\n"
                        
                 
#define HTTP_403_RESPONSE "HTTP/1.1 403 Forbidden\r\n" \
        "Date: Wed, 09 Oct 2024 15:34:48 GMT\r\n" \
        "Server: Apache/2.4.41 (Ubuntu)\r\n" \
        "Content-Type: text/html; charset=UTF-8\r\n" \
        "Content-Length: 345\r\n" \
        "Connection: close\r\n" \
        "\r\n" \
        "<html>\r\n" \
        "<head>\r\n" \
        "    <title>403 Forbidden</title>\r\n" \
        "</head>\r\n" \
        "<body>\r\n" \
        "    <h1>Forbidden</h1>\r\n" \
        "    <p>You don't have permission to access this resource.</p>\r\n" \
        "</body>\r\n" \
        "</html>\r\n" 

/**
 * @file http_helper.h
 * @brief Helper functions for handling HTTP requests.
 *
 * This file contains declarations for functions that help with the
 * parsing and validation of HTTP requests.
 */

int is_http_method(const char* buffer);
int is_http_request_complete(const char* buffer);
int get_http_host(const char* buffer, char* host, size_t host_size);

#endif
