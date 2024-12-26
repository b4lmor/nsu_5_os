#ifndef UTHREADPOOL_H
#define UTHREADPOOL_H

#include <stdio.h>
#include <sys/ucontext.h>

typedef enum uthread_status {
    IDLE,
    READY,
    RUNNING,
    TERMINATED
} uthread_status_t;

typedef struct {
    uthread_status_t status;
    ssize_t id;
    void (*func)(void *);
    void *arg;
    void *stack;
    ucontext_t context;
} uthread_t;

typedef struct uthread_node {
    struct uthread_node *next;
    struct uthread_node *prev;
    uthread_t *uthread;
} uthread_node_t;

typedef struct utrhead_scheduler {
    char is_finished;
    ssize_t id_generator;
    ssize_t size;
    uthread_node_t *head_uthread;
    uthread_node_t *active_uthread;
    ucontext_t initial_context;
} uthread_scheduler_t;

uthread_scheduler_t *init_scheduler();

int add_to_scheduler(uthread_scheduler_t *scheduler, void (*func)(void*), void *arg);

void usched_yield(uthread_scheduler_t *scheduler);

void run_scheduler(uthread_scheduler_t *scheduler);

void reset_scheduler(uthread_scheduler_t *scheduler);

void destroy_scheduler(uthread_scheduler_t **scheduler_ptr);

#endif
