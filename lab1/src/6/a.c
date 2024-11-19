#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "my_thread/my_thread.h"

int g = 0;

void *func(void *arg) {
    printf("%d\n", *(int *) arg);
    // printf("\n");
    g++;
    return 0;
}

void *f(void* arg) {
    sleep(1000);
}

#define N 100

int main() {
    printf("This process pid: %u\n", getpid());

    int *args = malloc(N * sizeof(int));
    my_thread_t threads[N];

    for (int i = 0; i < N; i++) {
        args[i] = i;
        int success = mythread_create(&threads[i], func, &args[i]);
        if (success == -1) {
            perror("mythread_create");
        }
        // sleep(1);
    }
    sleep(7);
    printf("count: %d\n", g);

    for (int i = 0; i < N; i++) {
        clear(&threads[i]);
    }

    sleep(1000);

    free(args);
    return EXIT_SUCCESS;
}
