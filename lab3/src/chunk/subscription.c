#include "../../include/subscription.h"
#include "../../include/log.h"

#include <pthread.h>
#include <unistd.h>

#include "../../include/common.h"

typedef struct subscription_info {
    subscriber_t *subscriber;
    subscription_manager_t *manager;
} subscription_info_t;

void *__send_chunks_to_subscriber(subscription_info_t *subscription_info);

void unsubscribe(subscription_manager_t *manager, subscriber_t **subscriber) {
    pthread_mutex_lock(&manager->lock);

    subscriber_t *next = (*subscriber)->next;
    subscriber_t *prev = (*subscriber)->prev;

    if (!next && !prev) {
        manager->first_subscriber = NULL;
        manager->last_subscriber = NULL;
        goto end;
    }
    if (!next) {
        manager->last_subscriber = prev;
        prev->next = NULL;
    } else {
        next->prev = prev;
    }
    if (!prev) {
        manager->first_subscriber = next;
        next->prev = NULL;
    } else {
        prev->next = next;
    }
end:
    pthread_mutex_lock(&(*subscriber)->lock);
    close((*subscriber)->fd);
    pthread_mutex_unlock(&(*subscriber)->lock);

    logis((*subscriber)->fd, "connection closed");

    free(*subscriber);

    *subscriber = NULL;

    manager->subscribers_count--;

    pthread_mutex_unlock(&manager->lock);
}

subscription_manager_t *create_subscription_manager() {
    subscription_manager_t *manager = malloc(sizeof(subscription_manager_t));
    if (!manager) {
        perror("Failed to allocate subscription manager");
        return NULL;
    }
    pthread_mutex_init(&manager->lock, NULL);
    pthread_cond_init(&manager->cond, NULL);
    manager->container = create_chunk_container();
    if (manager->container == NULL) {
        free(manager);
        return NULL;
    }
    manager->flag = 0;
    manager->subscribers_count = 0;
    manager->is_busy = 0;
    manager->first_subscriber = NULL;
    manager->last_subscriber = NULL;
    return manager;
}

void destroy_subscription_manager(subscription_manager_t **manager) {
    pthread_mutex_lock(&(*manager)->lock);
    free_chunk_container(&(*manager)->container);
    subscriber_t *subscriber = (*manager)->first_subscriber;
    while (subscriber) {
        subscriber_t *tmp = subscriber->next;
        unsubscribe(*manager, &subscriber);
        subscriber = tmp;
    }
    pthread_mutex_unlock(&(*manager)->lock);
    free(*manager);
    *manager = NULL;
}

int subscribe(subscription_manager_t *manager, const int fd) {
    pthread_mutex_lock(&manager->lock);

    subscriber_t *subscriber = malloc(sizeof(subscriber_t));
    if (subscriber == NULL) {
        return -1;
    }
    pthread_mutex_init(&subscriber->lock, NULL);
    subscriber->fd = fd;
    subscriber->chunk_loaded = 0;
    subscriber->next = NULL;
    subscriber->prev = manager->last_subscriber;
    if (manager->first_subscriber == NULL) {
        manager->first_subscriber = subscriber;
    } else {
        manager->last_subscriber->next = subscriber;
    }
    manager->last_subscriber = subscriber;

    manager->subscribers_count++;

    pthread_mutex_unlock(&manager->lock);

    return 0;
}

void finish_pending_chunks(subscription_manager_t *manager) {
    subscriber_t *subscriber = manager->first_subscriber;
    while (subscriber != NULL) {
        subscriber_t *tmp = subscriber->next;
        unsubscribe(manager, &subscriber);
        subscriber = tmp;
    }
}

// @synchronized
void add_chunk(subscription_manager_t *manager, const void *data, const size_t element_size, const size_t size,
               proxy_context_t *proxy_context) {
    pthread_mutex_lock(&manager->lock);
    __add_chunk(manager->container, data, element_size, size);
    __send_chunks(manager, proxy_context);
    // wait for all
    while (manager->flag > 0) {
        pthread_cond_wait(&manager->cond, &manager->lock);
    }
    pthread_mutex_unlock(&manager->lock);
}

void __send_chunks(subscription_manager_t *manager, const proxy_context_t *proxy_context) {
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

#ifdef PARALLEL
        threadpool_push_task(proxy_context->pool, __send_chunks_to_subscriber, info);
        info->manager->flag = info->manager->subscribers_count;
#else
        __send_chunks_to_subscriber(info);
        info->manager->flag = 0;
#endif
    next:
        current_subscriber = current_subscriber->next;
    }
}

// @parallel
// @synchronized
void *__send_chunks_to_subscriber(subscription_info_t *subscription_info) {
    subscription_manager_t *manager = subscription_info->manager;
    subscriber_t *subscriber = subscription_info->subscriber;

    pthread_mutex_lock(&subscriber->lock);

    const int start_chunk = subscriber->chunk_loaded;
    const int last_chunk = manager->container->count;

    const chunk_t *current_chunk = get_chunk(manager->container, start_chunk);
    if (!current_chunk) {
        goto end;
    }
    for (int i = start_chunk; i < last_chunk; i++) {
        if (send_data_to_fd(subscriber->fd, current_chunk->data, current_chunk->size * current_chunk->element_size)) {
            logis(subscriber->fd, "Error sending data to subscriber");
            goto end; // todo: unsubscribe ?
        }
        current_chunk = current_chunk->next;
        subscriber->chunk_loaded++;
    }
end:

    pthread_mutex_lock(&manager->lock);
    manager->flag--;
    pthread_cond_signal(&manager->cond);
    pthread_mutex_unlock(&manager->lock);

    pthread_mutex_unlock(&subscriber->lock);
    free(subscription_info);
    return NULL;
}
