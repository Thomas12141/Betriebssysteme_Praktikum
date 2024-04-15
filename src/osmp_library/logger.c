#include <stddef.h>
#include <stdio.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
FILE *logging_file;
char * file_name;

void logging_init(char * name){
    if(name == NULL||*name == '\0'){
        printf("You cant initialize logging file without giving a file name.");
        return;
    }
    file_name = name;
    logging_file = fopen(file_name, "a+");
    if(logging_file == NULL ){
        printf("Couldn't open logging file.\n");
    }
}

void log_to_file(char * level, char* message){
    int return_code;
    if(file_name == NULL){
        printf("You must first initialize the logger.\n");
        return;
    }
    return_code = pthread_mutex_lock(&mutex);
    if(return_code!=0){
        printf("Failed to acquire lock.\n");
        return;
    }
    fprintf(logging_file,"Level:%s  Time: %s Message: %s.\n", level, __TIMESTAMP__, message);
    return_code = pthread_mutex_unlock(&mutex);
    while (return_code!=0){
        printf("Failed to release lock.\n");
        return_code = pthread_mutex_unlock(&mutex);
    }
}


void logging_close(){
    fclose(logging_file);
    logging_file = NULL;
    pthread_mutex_destroy(&mutex);
}


