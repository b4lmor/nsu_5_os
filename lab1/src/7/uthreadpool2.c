#include "uthreadpool.h"

#include <stdlib.h>
#include <ucontext.h>
#include <sys/mman.h>

#define STACK_SIZE (1<<16)

int __create_uthread(uthread_scheduler_t *scheduler);

void __func_wrapper(uthread_scheduler_t *scheduler);

int __init_ucontext(uthread_t *uthread);

uthread_scheduler_t *init_scheduler() {
    uthread_scheduler_t *scheduler = malloc(sizeof(uthread_scheduler_t));
    if (scheduler == NULL) {
        perror("malloc failed");
        return NULL;
    }

    scheduler->is_finished = 0;
    scheduler->size = 0;
    scheduler->id_generator = 0;
    scheduler->active_uthread = NULL;

    return scheduler;
}

int add_to_scheduler(uthread_scheduler_t *scheduler, void (*func)(void *), void *arg) {
    const uthread_node_t *node = scheduler->head_uthread;
    int cnt = 0;
    while (cnt < scheduler->size && node->uthread->status != IDLE) {
        node = node->next;
        cnt++;
    }

    uthread_t *uthread;
    if (cnt >= scheduler->size) {
        if (__create_uthread(scheduler)) {
            return -1;
        }
        uthread = scheduler->head_uthread->uthread;
    } else {
        uthread = node->uthread;
    }

    uthread->arg = arg;
    uthread->func = func;
    uthread->status = READY;

    if (getcontext(&uthread->context)) {
        perror("getcontext failed");
        return -1;
    }

    makecontext(&uthread->context, __func_wrapper, 1, scheduler);

    return 0;
}

void usched_yield(uthread_scheduler_t *scheduler) {
    uthread_node_t *node = scheduler->active_uthread == NULL ? scheduler->head_uthread : scheduler->active_uthread;
    int cnt = 0;
    while (node->uthread->status != READY && cnt < scheduler->size - 1) {
        node = node->next;
        cnt++;
    }

    if (cnt >= scheduler->size - 1) {
        scheduler->is_finished = 1;
        setcontext(&scheduler->initial_context);
        return;
    }

    uthread_t *current_uthread = scheduler->active_uthread->uthread;

    if (current_uthread->status == TERMINATED) {
        current_uthread->status = IDLE;
    } else {
        current_uthread->status = READY;
    }

    scheduler->active_uthread = node;
    scheduler->active_uthread->uthread->status = RUNNING;

    swapcontext(&current_uthread->context, &node->uthread->context);
}

void run_scheduler(uthread_scheduler_t *scheduler) {
    getcontext(&scheduler->initial_context);

    if (scheduler->is_finished) {
        return;
    }

    uthread_node_t *node = scheduler->head_uthread;
    int cnt = 0;
    while (cnt < scheduler->size && node->uthread->status != READY) {
        node = node->next;
        cnt++;
    }

    if (cnt >= scheduler->size) {
        return;
    }

    scheduler->active_uthread = node;
    scheduler->active_uthread->uthread->status = RUNNING;

    setcontext(&node->uthread->context);
}

void reset_scheduler(uthread_scheduler_t *scheduler) {
    scheduler->is_finished = 0;
}

void destroy_scheduler(uthread_scheduler_t **scheduler_ptr) {
    uthread_scheduler_t *scheduler = *scheduler_ptr;
    uthread_node_t *node = scheduler->head_uthread;
    int cnt = 0;
    while (cnt < scheduler->size) {
        uthread_t *uthread = node->uthread;
        munmap(uthread->stack, STACK_SIZE);
        free(uthread);
        uthread_node_t *tmp = node->next;
        free(node);
        node = tmp;
        cnt++;
    }
    free(scheduler);
    *scheduler_ptr = NULL;
}

void __func_wrapper(uthread_scheduler_t *scheduler) {
    uthread_t *uthread = scheduler->active_uthread->uthread;

    uthread->func(uthread->arg);

    uthread->status = TERMINATED;

    usched_yield(scheduler);
}

int __init_ucontext(uthread_t *uthread) {
    uthread->stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (uthread->stack == MAP_FAILED) {
        perror("mmap failed");
        return -1;
    }

    uthread->context.uc_stack.ss_sp = uthread->stack;
    uthread->context.uc_stack.ss_size = STACK_SIZE;
    uthread->context.uc_link = 0;

    return 0;
}

int __create_uthread(uthread_scheduler_t *scheduler) {
    uthread_t *uthread = malloc(sizeof(uthread_t));
    if (uthread == NULL) {
        perror("malloc failed");
        return -1;
    }

    uthread->id = scheduler->id_generator++;
    uthread->status = IDLE;

    if (__init_ucontext(uthread)) {
        free(uthread);
        return -1;
    }

    uthread_node_t *node = malloc(sizeof(uthread_node_t));
    if (node == NULL) {
        perror("malloc failed");
        munmap(uthread->stack, STACK_SIZE);
        free(uthread);
        return -1;
    }

    node->uthread = uthread;

    if (scheduler->head_uthread == NULL) {
        node->prev = node;
        node->next = node;
    } else {
        node->next = scheduler->head_uthread->next;
        node->prev = scheduler->head_uthread;
        scheduler->head_uthread->next->prev = node;
        scheduler->head_uthread->next = node;
    }
    scheduler->head_uthread = node;

    scheduler->size++;

    return 0;
}
