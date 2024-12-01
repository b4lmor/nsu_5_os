#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>

#define MAX_THREADS 10

typedef struct task {
    void (*function)(void*);
    void *argument;
    struct task *next;
    struct task *prev;
} task_t;

typedef struct threadpool {
    int queue_size;
    int stop;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t threads[MAX_THREADS];
    task_t *last_task;
} threadpool_t;

threadpool_t* init_threadpool();

void destroy_threadpool(threadpool_t **pool);

void threadpool_push_task(threadpool_t *pool, void (*function)(void*), void *argument) ;

#endif
