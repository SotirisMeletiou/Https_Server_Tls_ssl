#ifndef SSL_QUEUE_H
#define SSL_QUEUE_H

#include <pthread.h>
#include <openssl/ssl.h>

typedef struct {
    SSL* ssl;
    int socket_fd;
} SSLConnection;

typedef struct Node {
    SSLConnection connection;
    struct Node* next;
} Node;

typedef struct {
    Node* front;
    Node* rear;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} SSL_QUEUE;

void initializeSSLQueue(SSL_QUEUE* queue);

void enqueueSSLConnection(SSL_QUEUE* queue, SSLConnection connection);

SSLConnection dequeueSSLConnection(SSL_QUEUE* queue);

void cleanupSSLQueue(SSL_QUEUE* queue);

void printQueue(const SSL_QUEUE* queue);

#endif
