#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
    int number;
    char* message;
} data_t;

void *mythread(void *arg) {
    printf("mythread [%d %d %d]: Hello from mythread!\n", getpid(), getppid(), gettid());
    data_t data = *(data_t*)arg;
    sleep(3);
    printf("data: %s, %d", data.message, data.number);
    return NULL;
}

int main() {
    data_t data;
    data.number = 10;
    data.message = malloc(20 * sizeof(char));
    strcpy(data.message, "hello world");

    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    int err = pthread_create(&tid, &attr, mythread, &data);
    if (err) {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        return -1;
    }

    // err = pthread_join(tid, NULL);
    // if (err) {
    //     printf("main: pthread_join() failed: %s\n", strerror(err));
    //     return -1;
    // }

    pthread_exit(NULL);

    free(data.message);

    return 0;
}
