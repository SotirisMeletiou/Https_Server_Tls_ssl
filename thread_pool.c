#include "thread_pool.h"
#include "https_methods.h"

void worker_thread (void* queue){
    SSL_QUEUE* ssl_queue = (SSL_QUEUE*) queue;
    while (1){
        SSLConnection connection = dequeueSSLConnection(ssl_queue);
        //usleep(10000000); //latency for testing, remove std c99 to work
        handle_connection(connection.ssl, connection.socket_fd);
    }
}

int init_thread_pool(pthread_t* pool, int num_threads, SSL_QUEUE* ssl_queue) {
    // Allocate memory for the pthread_t array
    pool = (pthread_t *) malloc(sizeof(pthread_t) * num_threads);
    if (pool == NULL) {
        // Handle error: Unable to allocate memory
        perror("Unable to allocate memory for thread pool: ");
        return 1;
    }

    // Create worker threads and store their IDs in the allocated array
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(pool + i, NULL, (void *) worker_thread, (void *) ssl_queue) != 0) {
            perror("Unable to create thread: ");
            return 1;
        }
    }
    return 0;
}

void destroy_thread_pool(pthread_t *pool, int num_threads) {
    for (int i = 0; i < num_threads; i++){
        pthread_cancel(pool[i]);
    }
    free(pool);
}
