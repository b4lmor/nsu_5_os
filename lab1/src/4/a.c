#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void *mythread(void *arg) {
    while (1) {
        printf("mythread [%d %d %d]: Hello from mythread!\n", getpid(), getppid(), gettid());
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t tid;

    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    int err = pthread_create(&tid, NULL, mythread, NULL);
    if (err) {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        return -1;
    }

    sleep(5);

    pthread_cancel(tid);

    pthread_join(tid, NULL);

    return 0;
}
