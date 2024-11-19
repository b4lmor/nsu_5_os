#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void *mythread(void *arg) {
    printf("mythread [%d %d %d]: Hello from mythread!\n", getpid(), getppid(), gettid());
    return NULL;
}

int main() {
    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    while (1) {
        pthread_t tid;
        int err = pthread_create(&tid, NULL, mythread, NULL);
        if (err) {
            printf("main: pthread_create() failed: %s\n", strerror(err));
            return EXIT_FAILURE;
        }
        // sleep(5);
    }

    return EXIT_SUCCESS;
}
