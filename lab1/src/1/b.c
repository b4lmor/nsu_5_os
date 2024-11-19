#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define N 5

int global = 1;

void *mythread(void *arg) {
    printf(
        "mythread #%d: [pid: %d | ppid: %d | tid: %d | self: %lu]\n",
        global, getpid(), getppid(), gettid(), pthread_self()
    );
    global++;
    // sleep(10000);
    return NULL;
}

int main() {
    printf("main:     [pid: %d | ppid: %d | tid: %d]\n", getpid(), getppid(), gettid());

    pthread_t tids[N];
    for (int i = 0; i < N; i++) {
        int err = pthread_create(&tids[i], NULL, mythread, NULL);
        if (err != 0) {
            printf("main: pthread_create() failed: %s\n", strerror(err));
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < N; i++) {
        int err = pthread_join(tids[i], NULL);
        if (err != 0) {
            printf("main: pthread_join() failed: %s\n", strerror(err));
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
