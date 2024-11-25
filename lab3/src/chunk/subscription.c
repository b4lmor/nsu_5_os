#include "../../include/subscription.h"

#include <stdatomic.h>

#include "../../include/common.h"

typedef struct subscription_info {
    subscriber_t *subscriber;
    subscription_manager_t *manager;
} subscription_info_t;

void *__send_chunks_to_subscriber(subscription_info_t *subscription_info);

void unsubscribe(subscription_manager_t *manager, const subscriber_t *subscriber) {
    subscriber_t *next = subscriber->next;
    subscriber_t *prev = subscriber->prev;
    if (!next && !prev) {
        manager->first_subscriber = NULL;
        manager->last_subscriber = NULL;
        return;
    }
    if (!next) {
        manager->last_subscriber = prev;
        prev->next = NULL;
    } else {
        next->prev = prev;
    }
    if (!prev) {
        manager->first_subscriber = next;
        next->next = NULL;
    } else {
        prev->next = next;
    }
}

subscription_manager_t *create_subscription_manager() {
    subscription_manager_t *manager = malloc(sizeof(subscription_manager_t));
    if (!manager) {
        return NULL;
    }
    manager->container = create_chunk_container();
    if (manager->container == NULL) {
        free(manager);
        return NULL;
    }
    manager->is_busy = 0;
    manager->first_subscriber = NULL;
    manager->last_subscriber = NULL;
    return manager;
}

void destroy_subscription_manager(subscription_manager_t **manager) {
    free_chunk_container(&(*manager)->container);
    subscriber_t *subscriber = (*manager)->first_subscriber;
    while (subscriber) {
        subscriber_t *tmp = subscriber->next;
        free(subscriber);
        subscriber = tmp;
    }
    free(*manager);
    *manager = NULL;
}

int subscribe(subscription_manager_t *manager, const int fd) {
    subscriber_t *subscriber = malloc(sizeof(subscriber_t));
    if (subscriber == NULL) {
        return -1;
    }
    subscriber->fd = fd;
    subscriber->chunk_loaded = 0;
    subscriber->next = NULL;
    subscriber->prev = NULL;
    if (manager->first_subscriber == NULL) {
        manager->first_subscriber = subscriber;
    } else {
        manager->last_subscriber->next = subscriber;
    }
    subscriber->prev = manager->last_subscriber;
    manager->last_subscriber = subscriber;

    if (manager->container->count != 0) {
        subscription_info_t *info = malloc(sizeof(subscription_info_t));
        if (info == NULL) {
            perror("malloc subscription_info_t");
            return 0;
        }
        info->subscriber = subscriber;
        info->manager = manager;
        __send_chunks_to_subscriber(info);
    }

    return 0;
}

void add_chunk(const subscription_manager_t *manager, const void *data, const size_t element_size, const size_t size) {
    __add_chunk(manager->container, data, element_size, size);
    __send_chunks(manager);
}

void finish_pending_chunks(subscription_manager_t *manager) {
    __send_chunks(manager);
    // todo : unsubscribe for everyone ?
}

int cas_state(subscription_manager_t *manager, char expected, char replaceto) {
    return atomic_compare_exchange_strong(&manager->is_busy, &expected, replaceto);
}

void __send_chunks(subscription_manager_t *manager) {
    subscriber_t *current_subscriber = manager->first_subscriber;
    while (current_subscriber != NULL) {
        if (current_subscriber->chunk_loaded == manager->container->count) {
            goto next;
        }
        subscription_info_t *info = malloc(sizeof(subscription_info_t));
        if (info == NULL) {
            perror("malloc subscription_info_t");
            goto next;
        }
        info->subscriber = current_subscriber;
        info->manager = manager;
        // todo: make parallel
        __send_chunks_to_subscriber(info);
    next:
        current_subscriber = current_subscriber->next;
    }
}

// @parallel
void *__send_chunks_to_subscriber(subscription_info_t *subscription_info) {
    const subscription_manager_t *manager = subscription_info->manager;
    subscriber_t *subscriber = subscription_info->subscriber;
    const int start_chunk = subscriber->chunk_loaded;
    const int last_chunk = manager->container->count;
    const chunk_t *current_chunk = get_chunk(manager->container, start_chunk);
    if (!current_chunk) {
        goto end;
    }
    for (int i = start_chunk; i < last_chunk; i++) {
        if (send_data_to_fd(subscriber->fd, current_chunk->data, current_chunk->size * current_chunk->element_size)) {
            printf("[%d] :: Error sending data to subscriber\n", subscriber->fd);
            goto end; // todo: unsubscribe ?
        }
        current_chunk = current_chunk->next;
        subscriber->chunk_loaded++;
    }
end:
    free(subscription_info);
    return NULL;
}
