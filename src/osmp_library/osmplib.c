/**
 * In dieser Quelltext-Datei sind Implementierungen der OSMP Bibliothek zu finden.
 */

#include "osmplib.h"

int OSMP_Init(const int *argc, char ***argv){
    return OSMP_CRITICAL_FAILURE;
}

int OSMP_Size(int *size){
    return OSMP_CRITICAL_FAILURE;
}

int OSMP_Rank(int *rank){
    return OSMP_CRITICAL_FAILURE;
}

int OSMP_Send(const void *buf, int count, OSMP_Datatype datatype, int dest){
    return OSMP_CRITICAL_FAILURE;
}

int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len){
    return OSMP_CRITICAL_FAILURE;
}

int OSMP_Finalize(void){
    return OSMP_CRITICAL_FAILURE;
}

int OSMP_Barrier(void){
    return OSMP_CRITICAL_FAILURE;
}

int OSMP_Bcast(void *buf, int count, OSMP_Datatype datatype, bool send, int *source, int *len){
    return OSMP_CRITICAL_FAILURE;
}

int OSMP_Isend(const void *buf, int count, OSMP_Datatype datatype, int dest, OSMP_Request request){
    return OSMP_CRITICAL_FAILURE;
}

int OSMP_Irecv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len, OSMP_Request request){
    return OSMP_CRITICAL_FAILURE;
}

int OSMP_Test(OSMP_Request request, int *flag){
    return OSMP_CRITICAL_FAILURE;
}

int OSMP_Wait (OSMP_Request request){
    return OSMP_CRITICAL_FAILURE;
}

int OSMP_CreateRequest(OSMP_Request *request){
    return OSMP_CRITICAL_FAILURE;
}

int OSMP_RemoveRequest(OSMP_Request *request){
    return OSMP_CRITICAL_FAILURE;
}

int OSMP_GetShmName(char** name){
    return OSMP_CRITICAL_FAILURE;
}