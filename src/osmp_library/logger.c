#include <stddef.h>
#include <stdio.h>

void log_to_file(void * logging_file, char * level, char* message){
    fprintf(logging_file,"Level:%s  Time: %s Message: %s.\n", level, __TIMESTAMP__, message);
}

int main (void) {
    FILE *logging_file;
    logging_file = fopen("log.log", "a+");

    if(logging_file == NULL ){
        printf("Couldn't open logging file.\n");
    }

    log_to_file(logging_file, "INFO", "File open success.");

    return 0;
}


