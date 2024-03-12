/**
 * In dieser Quelltext-Datei sind Implementierungen der OSMP Bibliothek zu finden.
 */

#include "osmplib.h"

int get_OSMP_MAX_PAYLOAD_LENGTH() {
  return OSMP_MAX_PAYLOAD_LENGTH;
}

int get_OSMP_MAX_SLOTS() {
    return OSMP_MAX_SLOTS;
}

int get_OSMP_MAX_MESSAGES_PROC() {
    return OSMP_MAX_MESSAGES_PROC;
}

int get_OSMP_NOT_IMPLEMENTED_YET() {
    return OSMP_NOT_IMPLEMENTED_YET;
}

int get_OSMP_FAILURE() {
    return OSMP_FAILURE;
}

int get_OSMP_SUCCESS() {
    return OSMP_SUCCESS;
}


int OSMP_Init(const int *argc, char ***argv) {
    UNUSED(argc);
    UNUSED(argv);
    return OSMP_NOT_IMPLEMENTED_YET;
}

int OSMP_SizeOf(OSMP_Datatype datatype) {
    UNUSED(datatype);
    return OSMP_NOT_IMPLEMENTED_YET;
}

int OSMP_Size(int *size) {
    UNUSED(size);
    return OSMP_NOT_IMPLEMENTED_YET;
}

int OSMP_Rank(int *rank) {
    UNUSED(rank);
    return OSMP_NOT_IMPLEMENTED_YET;
}

int OSMP_Send(const void *buf, int count, OSMP_Datatype datatype, int dest) {
    UNUSED(buf);
    UNUSED(count);
    UNUSED(datatype);
    UNUSED(dest);
    return OSMP_NOT_IMPLEMENTED_YET;
}

int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len) {
    UNUSED(buf);
    UNUSED(count);
    UNUSED(datatype);
    UNUSED(source);
    UNUSED(len);
    return OSMP_NOT_IMPLEMENTED_YET;
}

int OSMP_Finalize(void) {
    return OSMP_NOT_IMPLEMENTED_YET;
}

int OSMP_Barrier(void) {
    return OSMP_NOT_IMPLEMENTED_YET;
}

int OSMP_Gather(void *sendbuf, int sendcount, OSMP_Datatype sendtype, void *recvbuf, int recvcount, OSMP_Datatype recvtype, int recv) {
    UNUSED(sendbuf);
    UNUSED(sendcount);
    UNUSED(sendtype);
    UNUSED(recvbuf);
    UNUSED(recvcount);
    UNUSED(recvtype);
    UNUSED(recv);
    return OSMP_NOT_IMPLEMENTED_YET;
}

int OSMP_ISend(const void *buf, int count, OSMP_Datatype datatype, int dest, OSMP_Request request) {
    UNUSED(buf);
    UNUSED(count);
    UNUSED(datatype);
    UNUSED(dest);
    UNUSED(request);
    return OSMP_NOT_IMPLEMENTED_YET;
}

int OSMP_IRecv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len, OSMP_Request request) {
    UNUSED(buf);
    UNUSED(count);
    UNUSED(datatype);
    UNUSED(source);
    UNUSED(len);
    UNUSED(request);
    return OSMP_NOT_IMPLEMENTED_YET;
}

int OSMP_Test(OSMP_Request request, int *flag) {
    UNUSED(request);
    UNUSED(flag);
    return OSMP_NOT_IMPLEMENTED_YET;
}

int OSMP_Wait(OSMP_Request request) {
    UNUSED(request);
    return OSMP_NOT_IMPLEMENTED_YET;
}

int OSMP_CreateRequest(OSMP_Request *request) {
    UNUSED(request);
    return OSMP_NOT_IMPLEMENTED_YET;
}

int OSMP_RemoveRequest(OSMP_Request *request) {
    UNUSED(request);
    return OSMP_NOT_IMPLEMENTED_YET;
}

int OSMP_GetSharedMemoryName(char **name) {
    UNUSED(name);
    return OSMP_NOT_IMPLEMENTED_YET;
}