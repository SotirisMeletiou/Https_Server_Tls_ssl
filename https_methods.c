#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include "https_methods.h"

char* HTTP_HOME = NULL;

void initialize_http_home(char * http_home){
    HTTP_HOME = http_home;
}

// Function to check for "Connection: keep-alive" or "Connection: close" header
int checkConnectionHeader(const char *httpMessage) {
    // Search for "Connection:" header in the HTTP message
    const char *connectionHeader = strstr(httpMessage, "Connection: ");
    if (connectionHeader != NULL) {
        // Check for "keep-alive"
        const char *keepAliveToken = strstr(connectionHeader, "keep-alive");
        if (keepAliveToken != NULL) {
            return 1; // "Connection: keep-alive" found
        }

        // Check for "close"
        const char *closeToken = strstr(connectionHeader, "close");
        if (closeToken != NULL) {
            return 0; // "Connection: close" found
        }
    }
    // No "Connection:" header or neither "keep-alive" nor "close" found
    return 1;
}

void handle_connection(SSL *ssl, int socket_fd) {
    int bytes;
    char buf[8192];
    bytes = SSL_read(ssl, buf, sizeof(buf) - 1);
    buf[bytes] = '\0';
    char method[16], uri[256];
    sscanf(buf, "%s %s", method, uri);
    //if keep_alive is 1 or 0 thread must keep connection alive
    int keep_alive = checkConnectionHeader(buf);
    char* connection_header = NULL;
    //generate connection header literal
    if (keep_alive == 1){
        connection_header = "Connection: keep-alive\r\n";
    } else {
        connection_header = "Connection: close\r\n";
    }
    printf("Request: %s\n", buf);

    if (strcmp(method, "GET") == 0) {
        handle_get_request(ssl, uri, connection_header);
    } else if (strcmp(method, "POST") == 0) {
        handle_post_request(ssl, uri, buf, connection_header);
    } else if (strcmp(method, "HEAD") == 0) {
        handle_head_request(ssl, uri, connection_header);
    } else if (strcmp(method, "DELETE") == 0) {
        handle_delete_request(ssl, uri, connection_header);
    } else {
        char response[4096];
        sprintf(response, "HTTP/1.1 501 Not Implemented\r\n");
        sprintf(response + strlen(response), "Server: my_webserver\r\n");
        sprintf(response + strlen(response), "Connection: close\r\n");
        sprintf(response + strlen(response), "Content-type: text/plain\r\n");
        sprintf(response + strlen(response), "Content-Length: 24\r\n\r\n");
        sprintf(response + strlen(response), "Method not implemented!\r\n");
        
        SSL_write(ssl, response, strlen(response));
        printf("Response: %s\n", response);
    }

    printf("Response sent.\n\n\n");

    SSL_shutdown(ssl);
    SSL_free(ssl); 
    close(socket_fd);

}

const char* get_mime_type(const char *file_path) {
    const char *ext = strrchr(file_path, '.');

    if (!ext) {
        return "application/octet-stream";
    }

    if (strcmp(ext, ".txt") == 0 || strcmp(ext, ".sed") == 0 || strcmp(ext, ".awk") == 0 ||
        strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0) {
        return "text/plain";
    } else if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
        return "text/html";
    } else if (strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".jpg") == 0) {
        return "image/jpeg";
    } else if (strcmp(ext, ".gif") == 0) {
        return "image/gif";
    } else if (strcmp(ext, ".pdf") == 0) {
        return "application/pdf";
    } else if (strcmp(ext, ".json") == 0) {
        return "application/json";
    }
    return "application/octet-stream";
}

void handle_get_request(SSL *ssl, const char *request_uri, const char* connection_header) {
    char file_path[256];
    FILE *file;
    char response[4096];

    sprintf(file_path, "%s%s", HTTP_HOME, request_uri);
    printf("GET Request: %s\n", request_uri);
    file = fopen(file_path, "r");
    if (file != NULL) {
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        const char* content_type = get_mime_type(file_path);

        sprintf(response, "HTTP/1.1 200 OK\r\n");
        sprintf(response + strlen(response), "Server: my_webserver\r\n");
        sprintf(response + strlen(response), "%s", connection_header);
        sprintf(response + strlen(response), "Content-Length: %ld\r\n", file_size);
        sprintf(response + strlen(response), "Content-Type: %s\r\n\r\n", content_type);

        SSL_write(ssl, response, strlen(response));

        char buffer[1024];
        size_t bytes_read;

        while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            SSL_write(ssl, buffer, bytes_read);
        }

        fclose(file);
    } else {
        sprintf(response, "HTTP/1.1 404 Not Found\r\n");
        sprintf(response + strlen(response), "Server: my_webserver\r\n");
        sprintf(response + strlen(response), "%s\r\n", connection_header);

        SSL_write(ssl, response, strlen(response));
    }
    printf("Response: %s\n", response);

}

void handle_post_request(SSL *ssl, const char *request_uri, const char *request, const char* connection_header) {
    char file_path[256];
    FILE *file;
    char response[4096];

    sprintf(file_path, "%s%s", HTTP_HOME, request_uri);

    printf("POST Request: %s\n", request_uri);

    file = fopen(file_path, "w");
    if (file != NULL) {
        // Find the start of the request body
        char *body_start = strstr(request, "\r\n\r\n");
        if (body_start != NULL) {
            // Move past the double CRLF to the start of the body
            body_start += 4;

            // Write the body to the file
            fprintf(file, "%s", body_start);
        }

        fclose(file);

        sprintf(response, "HTTP/1.1 201 Created\r\n");
        sprintf(response + strlen(response), "Server: my_webserver\r\n");
        sprintf(response + strlen(response), "%s\r\n", connection_header);

        SSL_write(ssl, response, strlen(response));
    } else {
        sprintf(response, "HTTP/1.1 500 Internal Server Error\r\n");
        sprintf(response + strlen(response), "Server: my_webserver\r\n");
        sprintf(response + strlen(response), "%s\r\n", connection_header);

        SSL_write(ssl, response, strlen(response));
    }
    printf("Request Body: %s\n", request);
    printf("Response: %s\n", response);
}

void handle_head_request(SSL *ssl, const char *request_uri, const char* connection_header) {
    char file_path[256];
    char response[4096];

    sprintf(file_path, "%s%s", HTTP_HOME, request_uri);

    printf("HEAD Request: %s\n", request_uri);

    if (access(file_path, F_OK) != -1) {
        FILE *file = fopen(file_path, "r");
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        const char *content_type = get_mime_type(file_path);

        sprintf(response, "HTTP/1.1 200 OK\r\n");
        sprintf(response + strlen(response), "Server: my_webserver\r\n");
        sprintf(response + strlen(response), "%s", connection_header);
        sprintf(response + strlen(response), "Content-Length: %ld\r\n", file_size);
        sprintf(response + strlen(response), "Content-Type: %s\r\n\r\n", content_type);

        SSL_write(ssl, response, strlen(response));
        fclose(file);

    } else {
        sprintf(response, "HTTP/1.1 404 Not Found\r\n");
        sprintf(response + strlen(response), "Server: my_webserver\r\n");
        sprintf(response + strlen(response), "%s\r\n", connection_header);

        SSL_write(ssl, response, strlen(response));
    }
    printf("Response: %s\n", response);

}

void handle_delete_request(SSL *ssl, const char *request_uri, const char* connection_header) {
    char file_path[256];
    char response[4096];

    sprintf(file_path, "%s%s", HTTP_HOME, request_uri);

    printf("DELETE Request: %s\n", request_uri);

    if (access(file_path, F_OK) != -1) {
        sprintf(response, "HTTP/1.1 200 OK\r\n");
        sprintf(response + strlen(response), "Server: my_webserver\r\n");
        sprintf(response + strlen(response), "%s\r\n", connection_header);

        SSL_write(ssl, response, strlen(response));

        // Delete the file for DELETE request
        remove(file_path);
    } else {
        sprintf(response, "HTTP/1.1 404 Not Found\r\n");
        sprintf(response + strlen(response), "Server: my_webserver\r\n");
        sprintf(response + strlen(response), "%s\r\n", connection_header);

        SSL_write(ssl, response, strlen(response));
    }

    printf("Response: %s\n", response);
}

