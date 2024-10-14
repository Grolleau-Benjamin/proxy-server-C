/**
 * @file http_helper.h
 * @brief This file provides HTTP response macros for common HTTP status codes.
 *
 * The predefined HTTP response macros include the status code, headers, and a simple HTML body
 * for 404 Not Found, 401 Unauthorized, and 403 Forbidden responses. These can be used in server
 * applications to quickly send standardized responses to clients.
 */

#ifndef HTTP_HELPER
#define HTTP_HELPER

#include <stdlib.h>

/**
 * @brief HTTP 404 Not Found response.
 *
 * This macro defines a simple HTTP 404 response including headers and an HTML body.
 * The response includes a content type of "text/html" and a minimal HTML body displaying "404".
 */
#define HTTP_404_RESPONSE "HTTP/1.1 404 Not Found\r\n" \
                        "Content-Type: text/html\r\n" \
                        "Content-Length: 13\r\n" \
                        "\r\n" \
                        "<h1>404</h1>"

/**
 * @brief HTTP 403 Forbidden response.
 *
 * This macro defines a complete HTTP 403 Forbidden response, indicating that the user does not
 * have permission to access the requested resource. It includes headers such as the server
 * type, content type, and an HTML body with a message informing the client about the restriction.
 */                 
#define HTTP_403_RESPONSE "HTTP/1.1 403 Forbidden\r\n" \
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
