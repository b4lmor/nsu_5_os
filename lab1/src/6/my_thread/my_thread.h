#ifndef MY_THREAD_H
#define MY_THREAD_H

#define GUARD_SIZE (1 << 12)
#define STACK_SIZE (1 << 23)

typedef struct {
    char is_finished;
    int tid;
    void *stack;
    void *guard;
} my_thread_t;

int mythread_create(my_thread_t *thread, void *(thread_func)(void *), void *arg);

int clear(const my_thread_t *thread);

#endif //MY_THREAD_H
