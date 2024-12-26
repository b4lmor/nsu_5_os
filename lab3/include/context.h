#ifndef CONTEXT_H
#define CONTEXT_H

#include <stddef.h>
#include "hashmap.h"
#include "threadpool.h"

typedef struct {
    char stop;
    int port;
    hashmap_t *map;
    threadpool_t *handler_pool;
    threadpool_t *upload_pool;
} proxy_context_t;

proxy_context_t *init_context(int port, int upload_threads, int handler_threads);

void destroy_context(proxy_context_t **context);

#endif
