#ifndef CONTEXT_H
#define CONTEXT_H

#include <stddef.h>
#include "hashmap.h"
#include "threadpool.h"

typedef struct {
    char stop;
    int port;
    hashmap_t *map;
    threadpool_t *pool;
} proxy_context_t;

proxy_context_t *init_context(int port);

void destroy_context(proxy_context_t **context);

#endif
