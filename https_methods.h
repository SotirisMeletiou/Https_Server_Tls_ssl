#ifndef HTTPS_METHODS_H
#define HTTPS_METHODS_H

#include <openssl/ssl.h>

void initialize_http_home(char * http_home);    

const char *get_mime_type(const char *file_path);

void handle_get_request(SSL *ssl, const char *request_uri, const char* connection_header);

void handle_post_request(SSL *ssl, const char *request_uri, const char *request, const char* connection_header);

void handle_head_request(SSL *ssl, const char *request_uri, const char* connection_header);

void handle_delete_request(SSL *ssl, const char *request_uri, const char* connection_header);

void handle_connection(SSL *ssl, int file_fd);
#endif  