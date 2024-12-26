#include "../../include/threadpool.h"
#include "../../include/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* worker(void* arg);

void *worker(void *arg) {
    threadpool_t *pool = arg;

    while (1) {
        pthread_mutex_lock(&pool->mutex);

        while (pool->queue_size == 0 && !pool->stop) {
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }

        if (pool->stop) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }

        const task_t *task = pool->last_task;
        pool->queue_size--;
        pool->last_task = task->prev;

        pthread_mutex_unlock(&pool->mutex);

        logssi("POOL", "Starting task with id", task->id);
        task->function(task->argument);
        logssi("POOL", "Finished task with id", task->id);
    }

    return NULL;
}

threadpool_t *init_threadpool(int threads) {
    threadpool_t *pool = malloc(sizeof(threadpool_t));
    if (pool == NULL) {
        perror("malloc threadpool");
        return NULL;
    }

    pool->threads = malloc(sizeof(pthread_t) * threads);
    if (pool->threads == NULL) {
        perror("malloc pthread_t");
        free(pool);
        return NULL;
    }

    pool->queue_size = 0;
    pool->stop = 0;
    pool->last_task = NULL;

    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cond, NULL);

    for (int i = 0; i < MAX_THREADS; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker, pool)) {
            perror("pthread_create");
        }
    }

    return pool;
}

void threadpool_push_task(threadpool_t *pool, void (*function)(void *), void *argument) {
    pthread_mutex_lock(&pool->mutex);

    task_t *new_last_task = malloc(sizeof(task_t));
    new_last_task->function = function;
    new_last_task->argument = argument;
    new_last_task->next = NULL;
    new_last_task->prev = pool->last_task;
    new_last_task->id = pool->id_sequence++;
    if (pool->last_task != NULL) {
        pool->last_task->next = new_last_task;
    }
    pool->last_task = new_last_task;
    pool->queue_size++;
    pthread_cond_signal(&pool->cond);

    pthread_mutex_unlock(&pool->mutex);
}

void destroy_threadpool(threadpool_t **pool) {
    pthread_mutex_lock(&(*pool)->mutex);
    (*pool)->stop = 1;
    pthread_cond_broadcast(&(*pool)->cond);
    pthread_mutex_unlock(&(*pool)->mutex);

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join((*pool)->threads[i], NULL);
    }

    task_t *current = (*pool)->last_task;
    while (current != NULL) {
        task_t *tmp = current->prev;
        free(current);
        current = tmp;
    }

    free(*pool);
    *pool = NULL;
}