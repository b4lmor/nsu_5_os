#ifndef CONTEXT_H
#define CONTEXT_H

#include <stddef.h>
#include "hashmap.h"

typedef struct {
    int port;
    hashmap_t *map;
} proxy_context_t;

proxy_context_t *init_context(size_t mutex_number);

void destroy_context(proxy_context_t **context);

#endif
