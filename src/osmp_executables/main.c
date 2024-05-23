

#define _GNU_SOURCE
#include <stdio.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>

#include <unistd.h>

void* check(void * args){
    printf("thread tid %d pid %d\n", gettid(), getpid());
    return NULL;
}

int main(void) {
    pthread_t thread;
    pthread_create(&thread, NULL, check, NULL);
    printf("main tid %d pid %d\n", gettid(), getpid());
    pthread_join(thread, NULL);
    return 0;
}
