#include "my_thread.h"
#include <stdio.h>
#define __USE_GNU 1
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include <sys/mman.h>

typedef struct {
    my_thread_t *thread;
    void *(*func)(void *);
    void *arg;
} wrapper_data_t;

int *wrapper(void *arg) {
    wrapper_data_t* data = arg;
    data->func(data->arg);
    char *flag = &data->thread->is_finished;
    free(data);
    *flag = 1;
    return NULL;
}

int mythread_create(my_thread_t *thread, void *(thread_func)(void *), void *arg) {
    void *stack = mmap(NULL, STACK_SIZE + GUARD_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (stack == MAP_FAILED) {
        perror("mmap failed");
        return -1;
    }
    if (mprotect(stack, GUARD_SIZE, PROT_NONE) == -1) {
        perror("mprotect failed");
        munmap(stack, STACK_SIZE);
        return -1;
    }
    void *guard = stack;
    stack += GUARD_SIZE;

    thread->guard = guard;
    thread->stack = stack;

    wrapper_data_t *data = malloc(sizeof(wrapper_data_t));
    data->thread = thread;
    data->arg = arg;
    data->func = thread_func;

    thread->tid = clone(
        wrapper,
        stack + STACK_SIZE,
        CLONE_SIGHAND | CLONE_FS | CLONE_VM | CLONE_FILES | CLONE_THREAD,
        data
    );
    if (thread->tid == -1) {
        munmap(guard, GUARD_SIZE + STACK_SIZE);
    }
    return thread->tid;
}

int clear(const my_thread_t *thread) {
    if (!thread->is_finished) {
        printf("thread is still running! can't munmap its stack");
        return -1;
    }
    int err = munmap(thread->guard, STACK_SIZE + GUARD_SIZE);
    if (err == -1) {
        printf("mumnap failed");
        return -1;
    }
    printf("thread with tid=%d cleared\n", thread->tid);
    return 0;
}
