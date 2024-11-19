#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#define N 3

int global = 14;

void *mythread(void *arg) {
	printf(
		"mythread: [pid: %d | ppid: %d | tid: %d | self: %lu]\n",
		getpid(), getppid(), gettid(), pthread_self()
	);

	int mod = gettid() % 100;

	static int local_static = 11;
	int local = 12 * mod;
	const int local_const = 13 * mod;

	printf("\t\tLOCAL:        %p -> %d\n", &local, local);
	printf("\t\tLOCAL STATIC: %p -> %d\n", &local_static, local_static);
	printf("\t\tLOCAL CONST:  %p -> %d\n", &local_const, local_const);
	printf("\t\tGLOBAL:       %p -> %d\n", &global, global);

	printf("=-=-=- UPDATING VALUES ... -=-=-=\n");
	local++;
	global++;
	printf("\t\tLOCAL:        %p -> %d\n", &local, local);
	printf("\t\tGLOBAL:       %p -> %d\n", &global, global);

	return NULL;
}

int main() {
	printf("main:     [pid: %d | ppid: %d | tid: %d]\n\n", getpid(), getppid(), gettid());

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

	printf("\nmain:\tGLOBAL:       %p -> %d\n", &global, global);

	// for (int i = 0; i < N; i++) {
	// 	printf("\t%d. tid: %lu\n", i + 1, tids[i]);
	// }

	return EXIT_SUCCESS;
}
