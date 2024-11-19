#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define N 5

void *mythread(void *arg) {
    printf(
        "mythread: [pid: %d | ppid: %d | tid: %d | self: %lu]\n",
        getpid(), getppid(), gettid(), pthread_self()
    );
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

    for (int i = 0; i < N; i++) {
        printf("\t%d. tid: %lu\n", i + 1, tids[i]);
    }

    return EXIT_SUCCESS;
}
