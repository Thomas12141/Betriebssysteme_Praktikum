/**
 * In dieser Quelltext-Datei sind Implementierungen der OSMP Bibliothek zu finden.
 */
#define SHARED_MEMORY_NAME "/shared_memory"
#define SHARED_MEMORY_SIZE 1024

#include "osmplib.h"
#include "logger.h"
#include "OSMP.h"
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <malloc.h>

char *shm_ptr;
int shared_memory_fd, OSMP_size, OSMP_rank;
char *shared_memory_name;

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

/**
 * Offset (relativ zum Beginn des Shared Memory), ab dem der Postfach- und Nachrichten-Bereich beginnt, einschließlich
 * der Liste der freien Slots (vgl. Struktur des Shared Memory).
 * @return Offset (relativ zum Beginn des Shared Memory), an dem die Liste der freien Postfächer beginnt.
 */
int get_free_postboxes_list_offset() {
    log_osmp_lib_call(__TIMESTAMP__, "get_free_postboxes_list_offset");
    unsigned int int_size;
    OSMP_SizeOf(OSMP_INT, &int_size);
    // TODO: Berechne korrekten Offset für die Postfächer.
    // shm_size ints für die Ranks + 1 Mutex
    return (OSMP_size+1) *  (int)(int_size);
}

/**
 * Berechnet den Offset (relativ zum Beginn des Shared Memory), an dem der Bereich der Postfächer beginnt.
 * @return Offset (relativ zum Beginn des SHM), an dem die Postfächer beginnen.
 */
int get_postboxes_offset() {
    log_osmp_lib_call(__TIMESTAMP__, "get_postbox_offset");
    unsigned int int_size;
    OSMP_SizeOf(OSMP_INT, &int_size);
    return get_free_postboxes_list_offset() + OSMP_size * (int)int_size;
}

/**
 * Gibt den Offset (relativ zum Beginn des Shared Memory) zurück, an dem das Postfach des aufrufenden Prozesses liegt.
 * @return Offset (relativ zum Beginn des Shared Memory), an dem das Postfach des aufrufenden Prozesses liegt.
 */
int get_postbox_offset() {
    log_osmp_lib_call(__TIMESTAMP__, "get_postbox_offset");
    unsigned int int_size;
    OSMP_SizeOf(OSMP_INT, &int_size);
    return get_postboxes_offset() + OSMP_rank * (int)int_size;
}

/**
 * Gibt in *free_slots* die freien Nachrichtenslots zurück.
 * @param free_slots Zeiger auf ein int-Array, in das die freien Nachrichtenslots geschrieben werden. Leere Stellen des Arrays werden auf -1 gesetzt. Das Array muss OSMP_Size groß sein.
 */
void get_free_slots(int* free_slots) {
    log_osmp_lib_call(__TIMESTAMP__, "get_free_slots");
    memcpy(free_slots, shm_ptr + get_postboxes_offset(), (size_t)OSMP_size);
}

/**
 * Gibt den Offset zu dem Slot zurück, in dem die nächste Nachricht für den aufrufenden Prozess liegt.
 * @return Offset in Bytes zu dem Slot, in dem die nächste Nachricht für den Prozess liegt. NO_MESSAGE, wenn das Postfach leer ist.
 */
int get_next_message_slot() {
    log_osmp_lib_call(__TIMESTAMP__, "get_next_message_slot");
    int slot, offset;
    unsigned int int_size;
    OSMP_SizeOf(OSMP_INT, &int_size);
    offset = get_postbox_offset();
    memcpy(&slot, shm_ptr+offset+OSMP_rank, int_size);
    return slot;
}

/**
 * Prüft das Postfach des aufrufenden Prozesses auf die Anzahl der für ihn bereiten Nachrichten.
 * @return Die Anzahl der Nachrichten, die für den aufrufenden Prozess bereit sind.
 */
int get_number_of_messages() {
    log_osmp_lib_call(__TIMESTAMP__, "get_number_of_messages");
    int number = 0, offset, message_slot;
    unsigned int int_size;
    offset = get_postbox_offset();
    OSMP_SizeOf(OSMP_INT, &int_size);
    // Iteriere durch alle Einträge des Postfachs
    for(int i=0; i<OSMP_MAX_MESSAGES_PROC; i++) {
        memcpy(&message_slot, shm_ptr+offset, int_size);
        if(message_slot != NO_MESSAGE) {
            number++;
        }
    }
    return number;
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

    // Test
    sleep(3);
    printf("%s", shm_ptr);

    // Setze globale Variablen
    OSMP_Size(&OSMP_size);
    OSMP_Rank(&OSMP_rank);

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
    memcpy(size, shm_ptr, sizeof(int));
    printf("size: %d\n", *size);
    return OSMP_SUCCESS;
}

int OSMP_Rank(int *rank) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_Rank");
    char *iterator = shm_ptr + 5;
    char my_pid[20];
    int getpid_result = getpid();
    sprintf(my_pid, "%d", getpid_result);
    int osmp_size;
    OSMP_Size(&osmp_size);
    printf("Child PID: %s\n", my_pid);
    for (int i = 0; i < osmp_size; ++i) {
        printf("Child Iterator: %s\n", iterator);
        if(strcmp(my_pid, iterator) == 0){
            *rank = i;
            return OSMP_SUCCESS;
        }
        iterator += strlen(iterator) + 1;
    }
    printf("Rank not found\n");
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
