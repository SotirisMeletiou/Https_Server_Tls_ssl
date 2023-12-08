#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <unistd.h>
#include "ssl_queue.h"

int init_thread_pool(pthread_t* pool, int num_threads, SSL_QUEUE* ssl_queue);

void destroy_thread_pool(pthread_t *pool, int num_threads);

#endif 