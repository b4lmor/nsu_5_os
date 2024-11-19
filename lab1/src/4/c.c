#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void myfree(void* arg) {
    free(arg);
    printf("cleanup func\n");
}

void *mythread(void *arg) {
    char *msg = NULL;
    pthread_cleanup_push(myfree, msg);

    msg = malloc(1024);
    sprintf(msg, "Hello from thread %lu", pthread_self());

    while (1) {
        printf("%s\n", msg);
        sleep(1);
    }

    pthread_cleanup_pop(1);

    return NULL;
}

int main() {
    pthread_t tid;

    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    int err = pthread_create(&tid, NULL, mythread, NULL);
    if (err == -1) {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        return -1;
    }

    sleep(5);

    pthread_cancel(tid);

    pthread_join(tid, NULL);

    return 0;
}
