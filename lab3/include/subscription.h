#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#include <stddef.h>
#include "chunk.h"
#include "context.h"

typedef struct subscriber {
    int fd;
    size_t chunk_loaded;
    struct subscriber *next;
    struct subscriber *prev;
    pthread_mutex_t lock;
} subscriber_t;

typedef struct subscription_manager {
    char is_busy;
    char flag;
    size_t subscribers_count;
    subscriber_t *first_subscriber;
    subscriber_t *last_subscriber;
    chunk_container_t *container;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} subscription_manager_t;

subscription_manager_t *create_subscription_manager();

void destroy_subscription_manager(subscription_manager_t **manager);

int subscribe(subscription_manager_t *manager, int fd);

void unsubscribe(subscription_manager_t *manager, subscriber_t **subscriber);

void add_chunk(subscription_manager_t *manager, const void *data, size_t element_size, size_t size, proxy_context_t *proxy_context);

void finish_pending_chunks(subscription_manager_t *manager);

void __send_chunks(subscription_manager_t *manager, const proxy_context_t *proxy_context);

#endif
