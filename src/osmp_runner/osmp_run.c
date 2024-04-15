#include "osmp_run.h"
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

#define SHARED_MEMORY_NAME "/shared_memory"
#define SHARED_MEMORY_SIZE 1024

int string_to_int(const char * string){
    int result = 0;
    char iterator = string[0];
    int index = 0;
    while (iterator!='\0'){
        result = result*10 + iterator - '0';
        iterator = string[++index];
    }
    return result;
}

int get_executable_index(char ** arguments){
    char * iterator = arguments[0];
    int index = 0;
    while (iterator!=NULL&&iterator[0]!='.' && iterator[1]!='/'){
        iterator =  arguments[++index];
    }
    if(iterator==NULL){
        printf("There wasn't given any executable name.\n");
        return -1;
    }
    return index;
}

char ** get_all_arguments(char ** argv, int index){
    int count = 0;
    char * iterator = argv[index];
    int iterator_index = index;
    while (iterator!=NULL){
        count++;
        iterator = argv[++iterator_index];
    }
    char ** result = malloc(((unsigned long) (count + 1)) * sizeof(char*));
    if (result == NULL) {
        perror("Failed to malloc\n");
        exit(EXIT_FAILURE);
    }
    int result_index = 0;
    while (argv[index] != NULL) {
        result[result_index++] = argv[index++];
    }
    result[result_index] = NULL;
    return result;
}

int start_all_executables(int number_of_executables, int executable_index, char ** arguments){
    for (int i = 0; i < number_of_executables; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            printf("Failed to fork.\n");
            return -1;
        } else if (pid == 0) {
            execv(arguments[executable_index], arguments);
            perror("execv failed.\n");
            return -1;
        }
    }
    return 0;
}

int main (int argc, char **argv) {
    if (argc<2){
        printf("You have to give the number of executables and the executable file.\n");
        return -1;
    }

    int shared_memory_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);

    if (shared_memory_fd==-1){
        printf("Failed to open shared memory.\n");
        return -1;
    }
    int ftruncate_result = ftruncate(shared_memory_fd, SHARED_MEMORY_SIZE);

    if(ftruncate_result == -1){
        printf("Failed to truncate.\n");
        return -1;
    }

    int executable_index = get_executable_index(argv);
    int number_of_executables = string_to_int(argv[1]);
    char ** arguments = get_all_arguments(argv, executable_index+1);
    int starting_result = start_all_executables(number_of_executables, executable_index, argv);

    if(starting_result!=0){
        return -1;
    }

    for (int j = 0; j < number_of_executables; ++j) {
        int status;
        pid_t pid_child =  wait(&status);
        if (pid_child==-1){
            printf("Problem by waiting\n");
        }
        if ( WIFEXITED(status)&&WEXITSTATUS(status)!=0 ) {
            printf("Child returned failure code.\n");
        }
    }

    free(arguments);

    return 0;
}

