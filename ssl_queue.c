#include "ssl_queue.h"
#include <stdlib.h>

void initializeSSLQueue(SSL_QUEUE* queue) {
    queue->front = NULL;
    queue->rear = NULL;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
}

void enqueueSSLConnection(SSL_QUEUE* queue, SSLConnection connection) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->connection = connection;
    newNode->next = NULL;

    pthread_mutex_lock(&queue->mutex);

    if (queue->rear == NULL) {
        queue->front = newNode;
        queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }

    // Signal waiting threads that a new connection is available
    pthread_cond_signal(&queue->cond);
    fprintf(stderr, "Main thread id %lu\n", pthread_self());
    printQueue(queue);

    pthread_mutex_unlock(&queue->mutex);
}

SSLConnection dequeueSSLConnection(SSL_QUEUE* queue) {
    SSLConnection connection;
    Node* temp;

    pthread_mutex_lock(&queue->mutex);

    // Wait for a connection if the queue is empty
    while (queue->front == NULL) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    temp = queue->front;
    connection = temp->connection;
    queue->front = temp->next;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }

    free(temp);
    fprintf(stderr, "Thread id %lu\n", pthread_self());
    printQueue(queue);
    pthread_mutex_unlock(&queue->mutex);

    return connection;
}

void cleanupSSLQueue(SSL_QUEUE* queue) {
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);

    Node* current = queue->front;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }
}

// Function to print the contents of the queue
void printQueue(const SSL_QUEUE* queue) {
    struct Node* current = queue->front;
    int i = 0;
    fprintf(stderr, "Queue: \n");
    while (current != NULL) {
        fprintf(stderr, "node: %d fd: %d \n", i, current->connection.socket_fd);
        current = current->next;
        i++;
    }
}
