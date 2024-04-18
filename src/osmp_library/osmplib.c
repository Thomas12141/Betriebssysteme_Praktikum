/**
 * In dieser Quelltext-Datei sind Implementierungen der OSMP Bibliothek zu finden.
 */

#include <string.h>
#include <unistd.h>

#include "osmplib.h"
#include "logger.h"

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
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_Finalize");
    puts("OSMP_Finalize() not implemented yet");
    return OSMP_FAILURE;
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
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_GetSharedMemoryName");
    puts("OSMP_GetSharedMemoryName() not implemented yet");
    char * string = *name;
    int count = 0;
    while (string!=NULL){
        printf("%s", string);
        string = name[++count];
    }
    return OSMP_FAILURE;
}
