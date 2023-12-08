#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string.h>
#include "https_methods.h"
#include "ssl_queue.h"
#include "thread_pool.h"

int create_socket(int port, int queue_size) {
    int s;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    if (listen(s, queue_size) < 0) {
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }

    return s;
}

void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl() {
    EVP_cleanup();
}

SSL_CTX *create_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
    SSL_CTX_set_ecdh_auto(ctx, 1);

    /* Set the key and cert using dedicated pem files */
    if (SSL_CTX_use_certificate_file(ctx, "./keys/cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
	    exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "./keys/key.pem", SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
	    exit(EXIT_FAILURE);
    }
}

/*
    Reads a config file and returns the thread number, the port number and the http_home directory path by reference.
    @param filepath: the filepath of the config file
    @param threads: reference to the number of threads
    @param port: reference to the port number
    @param http_home: reference to http_home directory filepath
*/
void read_config_file(const char* filepath, int* threads, int* port, char** http_home) {
    FILE *config_file = fopen(filepath, "r");
    if (!config_file) {
        perror("Unable to open config file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), config_file) != NULL) {
        char key[32], value[256];
        if (sscanf(line, "%31[^=]=%255[^\n]", key, value) == 2) {
            if (strcmp(key, "THREADS") == 0) {
                *threads = atoi(value);
            } else if (strcmp(key, "PORT") == 0) {
                *port = atoi(value);
            } else if (strcmp(key, "HOME") == 0) {
                // Allocate memory for http_home and copy the value
                *http_home = (char*)malloc(strlen(value) + 1);
                if (*http_home == NULL) {
                    perror("Unable to allocate memory");
                    exit(EXIT_FAILURE);
                }
                strcpy(*http_home, value);
            }
        }
    }

    fclose(config_file);
}


int main() {
    int sock;
    SSL_CTX *ctx;
    //prepare thread pool
    int thread_num, port;
    char* http_home = NULL;
    read_config_file("./config.txt", &thread_num, &port, &http_home);
    printf("home:%s threads:%d port:%d\n", http_home, thread_num, port);
    init_openssl();
    ctx = create_context();
    configure_context(ctx);
    sock = create_socket(port, thread_num);
    //initialize home directory for resources
    initialize_http_home(http_home);
    //allocate space for the shared ssl connection queue
    SSL_QUEUE* ssl_queue = (SSL_QUEUE*) malloc(sizeof(SSL_QUEUE));
    //initialize the connection queue
    initializeSSLQueue(ssl_queue);
    //initialize the thread pool
    pthread_t * thread_pool = NULL;
    if (init_thread_pool(thread_pool, thread_num, ssl_queue) == 1){
        return EXIT_FAILURE;
    }

    while (1) {
        printf("Accepting client....\n");
        struct sockaddr_in addr;
        unsigned int len = sizeof(addr);
        SSL *ssl;

        int client = accept(sock, (struct sockaddr*)&addr, &len);
        if (client < 0) {
            perror("Unable to accept");
            exit(EXIT_FAILURE);
        }

        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        } else {
            SSLConnection newConnection = {ssl, client};
            enqueueSSLConnection(ssl_queue, newConnection);
        }

    }

    close(sock);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    //free thread resources
    destroy_thread_pool(thread_pool, thread_num);
    cleanupSSLQueue(ssl_queue);
    free(ssl_queue);
    free(http_home);
    return 0;
}