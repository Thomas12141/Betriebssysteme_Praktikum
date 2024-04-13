#include <stddef.h>
#include <stdio.h>

void log_to_file(void * fp, char * level, char* message){
    fprintf(fp,"Level:%s  Time: %s Message: %s.\n", level, __TIMESTAMP__, message);
}

int main (void) {
    FILE *fp;
    fp = fopen("log.log", "a+");

    if(fp == NULL ){
        printf("Couldn't open logging file.\n");
    }

    log_to_file(fp, "INFO", "File open success.");

    return 0;
}


