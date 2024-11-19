#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>

#define MAX_THREADS 10
#define STACK_SIZE 1024 * 64

typedef struct {
    ucontext_t context;
    void (*start_routine)(void*);
    void *arg;
    char stack[STACK_SIZE];
    int is_active;
} uthread_t;

uthread_t threads[MAX_THREADS];
int current_thread = -1;

void uthread_yield() {
    int prev_thread = current_thread;
    current_thread = (current_thread + 1) % MAX_THREADS;
    while (threads[current_thread].is_active == 0) {
        current_thread = (current_thread + 1) % MAX_THREADS;
        if (current_thread == prev_thread) {
            return;
        }
    }
    swapcontext(&threads[prev_thread].context, &threads[current_thread].context);
}

void thread_wrapper() {
    threads[current_thread].start_routine(threads[current_thread].arg);
    threads[current_thread].is_active = 0;
    uthread_yield();
}

int uthread_create(uthread_t *uthread, void *start_routine, void *arg) {
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i].is_active == 0) {
            threads[i].is_active = 1;
            threads[i].start_routine = start_routine;
            threads[i].arg = arg;
            getcontext(&threads[i].context);
            threads[i].context.uc_stack.ss_sp = threads[i].stack;
            threads[i].context.uc_stack.ss_size = sizeof(threads[i].stack);
            threads[i].context.uc_link = 0;
            makecontext(&threads[i].context, thread_wrapper, 0);
            *uthread = threads[i];
            return i;
        }
    }
    return -1;
}

void example_function(void *arg) {
    int *num = arg;
    for (int i = 0; i < 5; i++) {
        printf("Thread with number: %d, iteration: %d\n", *num, i);
        sleep(1);
        uthread_yield();
    }
}

int main() {
    int num1 = 1, num2 = 2;
    uthread_t ut1, ut2;

    int success;

    success = uthread_create(&ut1, example_function, &num1);
    if (success == -1) {
        perror("uthread_create");
    }
    success = uthread_create(&ut2, example_function, &num2);
    if (success == -1) {
        perror("uthread_create");
    }

    current_thread = 0;
    setcontext(&threads[current_thread].context);

    printf("back to main()\n");

    return EXIT_SUCCESS;
}
