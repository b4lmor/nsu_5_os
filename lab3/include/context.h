#ifndef CONTEXT_H
#define CONTEXT_H

#include <stddef.h>
#include <pthread.h>

typedef struct {
    size_t mutex_number;
    pthread_mutex_t *mutexes;
    char **last_cached_urls;
} Context;

Context *init_context(size_t mutex_number);

void destroy_context(Context **context);

#endif
