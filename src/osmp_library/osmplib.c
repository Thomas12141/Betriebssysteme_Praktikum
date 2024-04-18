/**
 * In dieser Quelltext-Datei sind Implementierungen der OSMP Bibliothek zu finden.
 */
#define SHARED_MEMORY_NAME "/shared_memory"
#define SHARED_MEMORY_SIZE 1024

#include "osmplib.h"
#include "logger.h"
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <bits/fcntl.h>
#include <malloc.h>

#include "osmplib.h"
#include "logger.h"

char *shm_ptr;
int shared_memory_fd;
char * shared_memory_name;

/**
 * Übergibt eine Level-1-Lognachricht an den Logger.
 *
 * @param pid           Die Process ID des aufrufenden Prozesses.
 * @param timestamp     Der Zeitpunkt des Aufrufs
 * @param function_name Der Name der aufrufenden Funktion.
 */
void log_osmp_lib_call(char* timestamp, const char* function_name) {
    // ausreichend großen Buffer für formatierten String erstellen
    unsigned long string_len = 30 + strlen(timestamp) + strlen(function_name);
    char message[string_len];
    sprintf(message, "OSMP function %s() called", function_name);
    log_to_file(1, timestamp, message);
}

int get_OSMP_MAX_PAYLOAD_LENGTH(void) {
    log_osmp_lib_call(__TIMESTAMP__, "get_OSMP_MAX_PAYLOAD_LENGTH");
    return OSMP_MAX_PAYLOAD_LENGTH;
}

int get_OSMP_MAX_SLOTS(void) {
    log_osmp_lib_call(__TIMESTAMP__, "get_OSMP_MAX_SLOTS");
    return OSMP_MAX_SLOTS;
}

int get_OSMP_MAX_MESSAGES_PROC(void) {
    log_osmp_lib_call(__TIMESTAMP__, "get_OSMP_MAX_MESSAGES_PROC");
    return OSMP_MAX_MESSAGES_PROC;
}

int get_OSMP_FAILURE(void) {
    log_osmp_lib_call(__TIMESTAMP__, "get_OSMP_FAILURE");
    return OSMP_FAILURE;
}

int get_OSMP_SUCCESS(void) {
    log_osmp_lib_call(__TIMESTAMP__, "get_OSMP_SUCCESS");
    return OSMP_SUCCESS;
}


int OSMP_Init(const int *argc, char ***argv) {
    OSMP_GetSharedMemoryName(&shared_memory_name);
    shared_memory_fd = shm_open(shared_memory_name,O_CREAT | O_RDWR, 0666);
    if(shared_memory_fd == -1){
        log_to_file(3, __TIMESTAMP__, "Failed to open shared memory.\n");
        return OSMP_FAILURE;
    }
    shm_ptr = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    if(shm_ptr == MAP_FAILED){
        log_to_file(3, __TIMESTAMP__, "Failed to map shared memory.\n");
        return OSMP_FAILURE;
    }
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_Init");
    for (int i = 0; i < *argc; ++i) {
        printf("%s ", (*argv)[i]);
    }
    puts("");
    return OSMP_SUCCESS;
}

int OSMP_SizeOf(OSMP_Datatype datatype, unsigned int *size) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_SizeOf");
    if(datatype == OSMP_SHORT){
        *size = sizeof(short int);
        return OSMP_SUCCESS;
    }else if(datatype == OSMP_INT){
        *size = sizeof(int);
        return OSMP_SUCCESS;
    }else if(datatype == OSMP_LONG){
        *size = sizeof(long int);
        return OSMP_SUCCESS;
    }else if(datatype == OSMP_UNSIGNED_CHAR){
        *size = sizeof(unsigned char);
        return OSMP_SUCCESS;
    }else if(datatype == OSMP_UNSIGNED){
        *size = sizeof(unsigned);
        return OSMP_SUCCESS;
    }else if(datatype == OSMP_UNSIGNED_SHORT){
        *size = sizeof(unsigned short int);
        return OSMP_SUCCESS;
    }else if(datatype == OSMP_UNSIGNED_LONG){
        *size = sizeof(unsigned long int);
        return OSMP_SUCCESS;
    }else if(datatype == OSMP_FLOAT){
        *size = sizeof(float);
        return OSMP_SUCCESS;
    }else if(datatype == OSMP_DOUBLE){
        *size = sizeof(double);
        return OSMP_SUCCESS;
    }else if(datatype == OSMP_BYTE){
        *size = sizeof(char );
        return OSMP_SUCCESS;
    }
    return OSMP_FAILURE;
}

int OSMP_Size(int *size) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_Size");
    printf("%d\n", *size);
    return OSMP_FAILURE;
}

int OSMP_Rank(int *rank) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_Rank");
    printf("%d\n", *rank);
    return OSMP_FAILURE;
}

int OSMP_Send(const void *buf, int count, OSMP_Datatype datatype, int dest) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_Send");
    puts("OSMP_Send() not implemented yet");
    UNUSED(buf);
    UNUSED(count);
    UNUSED(datatype);
    UNUSED(dest);
    return OSMP_FAILURE;
}

int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_Recv");
    puts("OSMP_Recv() not implemented yet");
    UNUSED(buf);
    UNUSED(count);
    UNUSED(datatype);
    UNUSED(source);
    UNUSED(len);
    return OSMP_FAILURE;
}

int OSMP_Finalize(void) {
    int result = close(shared_memory_fd);
    if(result == -1){
        log_to_file(3, __TIMESTAMP__, "Couldn't close file descriptor memory.");
        return OSMP_FAILURE;
    }
    free(shared_memory_name);
    result = munmap(shm_ptr, SHARED_MEMORY_SIZE);
    if(result==-1){
        log_to_file(3, __TIMESTAMP__, "Couldn't unmap memory.");
        return OSMP_FAILURE;
    }
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_Finalize");
    puts("OSMP_Finalize() not implemented yet");
    return OSMP_SUCCESS;
}

int OSMP_Barrier(void) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_Barrier");
    puts("OSMP_Barrier() not implemented yet");
    return OSMP_FAILURE;
}

int OSMP_Gather(void *sendbuf, int sendcount, OSMP_Datatype sendtype, void *recvbuf, int recvcount, OSMP_Datatype recvtype, int recv) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_Gather");
    puts("OSMP_Gather() not implemented yet");
    UNUSED(sendbuf);
    UNUSED(sendcount);
    UNUSED(sendtype);
    UNUSED(recvbuf);
    UNUSED(recvcount);
    UNUSED(recvtype);
    UNUSED(recv);
    return OSMP_FAILURE;
}

int OSMP_ISend(const void *buf, int count, OSMP_Datatype datatype, int dest, OSMP_Request request) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_ISend");
    puts("OSMP_ISend() not implemented yet");
    UNUSED(buf);
    UNUSED(count);
    UNUSED(datatype);
    UNUSED(dest);
    UNUSED(request);
    return OSMP_FAILURE;
}

int OSMP_IRecv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len, OSMP_Request request) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_IRecv");
    puts("OSMP_IRecv() not implemented yet");
    UNUSED(buf);
    UNUSED(count);
    UNUSED(datatype);
    UNUSED(source);
    UNUSED(len);
    UNUSED(request);
    return OSMP_FAILURE;
}

int OSMP_Test(OSMP_Request request, int *flag) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_Test");
    puts("OSMP_Test not implemented yet");
    UNUSED(request);
    UNUSED(flag);
    return OSMP_FAILURE;
}

int OSMP_Wait(OSMP_Request request) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_Wait");
    puts("OSMP_Wait() not implemented yet");
    UNUSED(request);
    return OSMP_FAILURE;
}

int OSMP_CreateRequest(OSMP_Request *request) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_CreateRequest");
    puts("OSMP_CreateRequest() not implemented yet");
    UNUSED(request);
    return OSMP_FAILURE;
}

int OSMP_RemoveRequest(OSMP_Request *request) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_RemoveRequest");
    puts("OSMP_RemoveRequest) not implemented yet");
    UNUSED(request);
    return OSMP_FAILURE;
}

int OSMP_GetSharedMemoryName(char **name) {
    int parent = getppid();
    int size = snprintf(NULL, 0, "%s_%d" ,SHARED_MEMORY_NAME , parent);
    *name = malloc(((unsigned long) (size + 1)) * sizeof(char));
    int result = sprintf(*name,"%s_%d" ,SHARED_MEMORY_NAME , parent);
    if(result<0){
        return OSMP_FAILURE;
    }
    return OSMP_SUCCESS;
}
