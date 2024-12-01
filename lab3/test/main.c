#include "../include/threadpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void sample_task(void *arg) {
    const int *num = arg;
    printf("Task %d is being executed\n", *num);
    sleep(1);
}

int main() {
    threadpool_t *pool = init_threadpool();

    for (int i = 0; i < 200; i++) {
        int *arg = malloc(sizeof(*arg));
        *arg = i;
        threadpool_push_task(pool, sample_task, arg);
    }

    sleep(22);
    destroy_threadpool(&pool);
    return 0;
}
