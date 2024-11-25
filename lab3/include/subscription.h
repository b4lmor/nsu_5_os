#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#include <stddef.h>
#include "chunk.h"

typedef struct subscriber {
    int fd;
    size_t chunk_loaded;
    struct subscriber *next;
    struct subscriber *prev;
} subscriber_t;

typedef struct subscription_manager {
    char is_busy;
    subscriber_t *first_subscriber;
    subscriber_t *last_subscriber;
    chunk_container_t *container;
} subscription_manager_t;

subscription_manager_t *create_subscription_manager();

void destroy_subscription_manager(subscription_manager_t **manager);

int subscribe(subscription_manager_t *manager, int fd);

void add_chunk(const subscription_manager_t *manager, const void *data, size_t element_size, size_t size);

void finish_pending_chunks(subscription_manager_t *manager);

int cas_state(subscription_manager_t *manager, char expected, char replaceto);

void __send_chunks(const subscription_manager_t *manager);

#endif
