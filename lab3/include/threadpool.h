#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <stdint.h>

#define MAX_THREADS 10

typedef struct task {
    int id;
    void (*function)(void*);
    void *argument;
    struct task *next;
    struct task *prev;
} task_t;

typedef struct threadpool {
    int queue_size;
    int stop;
    int max_nested;
    int nested_count;
    int id_sequence;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t *threads;
    task_t *last_task;
} threadpool_t;

threadpool_t* init_threadpool(int threads);

void destroy_threadpool(threadpool_t **pool);

void threadpool_push_task(threadpool_t *pool, void (*function)(void*), void *argument) ;

#endif
