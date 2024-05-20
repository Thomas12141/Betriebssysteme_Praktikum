/**
 * In dieser Quelltext-Datei sind Implementierungen der OSMP Bibliothek zu finden.
 */
#define SHARED_MEMORY_NAME "/shared_memory"
#define _GNU_SOURCE

#include "osmplib.h"
#include "logger.h"
#include "OSMP.h"
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <malloc.h>
#include <pthread.h>
#include <stdlib.h>

shared_memory *shm_ptr;
int shared_memory_fd, OSMP_size, OSMP_rank, memory_size;

/**
 * Lockt den angegebenen Mutex.
 * @param mutex Zeiger auf den Mutex, der zur Synchronisierung verwendet werden soll.
 */
void semwait(pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
}

/**
 * Gibt einen Mutex frei.
 * @param mutex Zeiger auf den freizugebenen Mutex.
 */
void semsignal(pthread_mutex_t* mutex) {
    pthread_mutex_unlock(mutex);
}

/**
 * Übergibt eine Level-1-Lognachricht an den Logger.
 *
 * @param pid           Die Process ID des aufrufenden Prozesses.
 * @param function_name Der Name der aufrufenden Funktion.
 */
void log_osmp_lib_call( const char* function_name) {
    // ausreichend großen Buffer für formatierten String erstellen
    unsigned long string_len = 30 + strlen(function_name);
    char message[string_len];
    sprintf(message, "OSMP function %s() called", function_name);
    log_to_file(1, message);
}

/**
 * Gibt die Nummer des nächsten freien Nachrichtenslots zurück. Dieser wird aus der Liste der freien Slots gelöscht,
 * alle übrigen Einträge im Array rücken eine Stelle nach vorne.
 * Das Flag im Slot selbst wird dadurch noch nicht auf "belegt" gesetzt! Dafür ist der Aufrufende verantwortlich.
 * @return Die Nummer des nächsten freien Nachrichtenslots. NO_SLOT, wenn kein Slot frei ist.
 */
int get_next_free_slot(void) {
    log_osmp_lib_call("get_next_free_slot");
    //Das Ergebnis.
    int slot;
    //Index für den Freien Slots Array.
    int index;
    //Kritischer Abschnitt
    semwait(&shm_ptr->mutex_shm_free_slots);
    //Den index für den Array.
    sem_getvalue(&shm_ptr->sem_shm_free_slots, &index);
    //Runterzählen
    sem_post(&shm_ptr->sem_shm_free_slots);
    slot = shm_ptr->free_slots[index];
    semsignal(&shm_ptr->mutex_shm_free_slots);

    return slot;
}

/**
 * Gibt einen Zeiger auf das process_info-Struct des angegebenen Prozesses zurück.
 * @param rank Rang des Prozesses, dessen process_info angefordert wird.
 * @return Zeiger auf process_info-Struct des Prozesses mit dem angegebenen Rang.
 */
process_info* get_process_info(int rank) {
    log_osmp_lib_call("get_process_info");
    // Prozess-Info 0
    process_info* info = &(shm_ptr->first_process_info);
    // Offset zum passenden Rang
    info += rank;
    return info;
}

/**
 * Gibt einen Zeiger auf den Nachrichtenslot zurück, in dem die nächste Nachricht für den angegebenen Prozess liegt.
 * @param rank Rang des Prozesses, dessen erster Nachrichtenslot zurückgegeben werden soll.
 * @return Zeiger auf den Slot, in dem die nächste Nachricht für den Prozess mit Rang *rank* liegt. Bei einem Fehler von malloc wird das Prozess mit OSMP_FAILURE abgebrochen.
 */
message_slot* get_next_message_slot(int rank, int * message_offset) {
    log_osmp_lib_call("get_next_message_slot");
    process_info* process = get_process_info(rank);
    sem_wait(&process->postbox.sem_proc_full);
    semwait(&process->postbox.mutex_proc_out);
    int out_index = process->postbox.out_index;
    *message_offset = process->postbox.postbox[out_index];
    process->postbox.postbox[out_index] = NO_MESSAGE;
    --process->postbox.out_index;
    if(process->postbox.out_index<0){
        process->postbox.out_index= OSMP_MAX_MESSAGES_PROC-1;
    }
    semsignal(&process->postbox.mutex_proc_out);
    sem_post(&process->postbox.sem_proc_empty);
    return &shm_ptr->slots[*message_offset];
}

/**
 * Setzt die globalen Variablen der OSMP-Bibliothek für den Elternprozess.
 * @param fd    Shared-Memory-File-Descriptor.
 * @param shm   Zeiger auf den Beginn des Shared Memory.
 * @param size  Größe des Shared Memory in Bytes.
 */
void OSMP_Init_Runner(int fd, shared_memory* shm, int size) {
    shared_memory_fd = fd;
    memory_size = size;
    shm_ptr = shm;
    OSMP_Size(&OSMP_size);
}

/**
 * Berechnet den für den Shared Memory benötigten Speicherplatz in Abhängigkeit von der Anzahl der Prozesse.
 * @param processes Die Anzahl der Executable-Prozesse, die verwaltet werden.
 * @return Die Größe des benötigten Speicherplatzes in Bytes.
 */
int calculate_shared_memory_size(int processes) {
    int size = (int)sizeof(shared_memory);
    // Das Struct enthält bereits Speicher für einen Prozess; nur der nötige Speicher für die weiteren n-1 Prozesse muss
    // noch addiert werden.
    size += (processes-1) * (int)sizeof(process_info);
    return size;
}

/**
 * Interne Implementierung der OSMP_Barrier()-Funktion. Der Hauptteil wird mithilfe der Posix-Funktion
 * pthread_cond_wait() implementiert, die eine Voraussetzung (Prediction) und eine Bedingung (Condition) benötigt.
 * @param barrier Zeiger auf die Barriere, an der gewartet werden soll.
 * @return Im Erfolgsfall OSMP_SUCCESS, sonst OSMP_FAILURE.
 */
int barrier_wait(barrier_t* barrier) {
    int status, cancel, tmp, cycle;

    if(barrier->valid != BARRIER_VALID) {
        log_to_file(3, "Barrier not yet initialized, can't wait at barrier!");
        return OSMP_FAILURE;
    }

    status = pthread_mutex_lock(&(barrier->mutex));
    if(status != 0) {
        log_to_file(3, "Failed to lock barrier mutex!");
        return OSMP_FAILURE;
    }

    cycle = barrier->cycle; // aktuellen Zyklus merken

    // Thread-Safety: nur die Hauptthreads der ursprünglichen OSMP-Prozesse dürfen den Counter beeinflussen
    if(getpid()==gettid()) {
        (barrier->counter)--;
    }
    if(barrier->counter== 0) {
        // Der letzte Thread in der Barriere initialisiert die Barrier für den nächsten Durchlauf.
        barrier->counter = OSMP_size;
        (barrier->cycle)++;
        pthread_cond_broadcast(&(barrier->convar));
    } else {
        // Da barrier_wait() kein Abbruchpunkt sein sollte, wird Abbruch deaktiviert.
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cancel);

        /* Warten, bis sich die Zyklusnummer der Voraussetzung (Prediction) ändert, was bedeutet, dass nicht länger
         * gewartet werden soll und die Bedingung (Condition) erfüllt ist. */
        while(cycle == barrier->cycle) {
            status = pthread_cond_wait(&(barrier->convar), &(barrier->mutex));
            if(status != 0) {
                break;
            }
        }

        pthread_setcancelstate(cancel, &tmp);
    }

    pthread_mutex_unlock(&(barrier->mutex));
    return OSMP_SUCCESS;
}

int get_OSMP_MAX_PAYLOAD_LENGTH(void) {
    log_osmp_lib_call("get_OSMP_MAX_PAYLOAD_LENGTH");
    return OSMP_MAX_PAYLOAD_LENGTH;
}

int get_OSMP_MAX_SLOTS(void) {
    log_osmp_lib_call("get_OSMP_MAX_SLOTS");
    return OSMP_MAX_SLOTS;
}

int get_OSMP_MAX_MESSAGES_PROC(void) {
    log_osmp_lib_call("get_OSMP_MAX_MESSAGES_PROC");
    return OSMP_MAX_MESSAGES_PROC;
}

int get_OSMP_FAILURE(void) {
    log_osmp_lib_call("get_OSMP_FAILURE");
    return OSMP_FAILURE;
}

int get_OSMP_SUCCESS(void) {
    log_osmp_lib_call("get_OSMP_SUCCESS");
    return OSMP_SUCCESS;
}

int OSMP_Init(const int *argc, char ***argv) {
    if(gettid() != getpid()){
        printf("Initializing of threads is not allowed\n");
        exit(0);
    }
    char *shared_memory_name = calloc(MAX_PATH_LENGTH, sizeof(char));
    OSMP_GetSharedMemoryName(&shared_memory_name);
    shared_memory_fd = shm_open(shared_memory_name,O_RDWR, 0666);
    if(shared_memory_fd == -1){
        printf("Failed to open shared memory.\n");
        return OSMP_FAILURE;
    }

    // Mappe zunächst nur die feste Größe des Shared Memory, um die Anzahl der Prozesse auszulesen
    shm_ptr = mmap(NULL, (size_t)sizeof(shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    int processes;
    processes = shm_ptr->size;
    // Unmap
    munmap(shm_ptr, (int)sizeof(shared_memory));

    // Berechne die tatsächliche Größe des Shared Memory
    memory_size = calculate_shared_memory_size(processes);
    // Mappe neu mit der passenden Größe
    shm_ptr = mmap(NULL, (size_t)memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);

    logging_init_child(shm_ptr);

    log_osmp_lib_call("OSMP_Init");
    log_to_file(2, "Calloc 256 B (shared_memory_name)");
    if(shm_ptr == MAP_FAILED){
        log_to_file(3, "Failed to map shared memory.\n");
        return OSMP_FAILURE;
    }

    for (int i = 0; i < *argc; ++i) {
        printf("%s ", (*argv)[i]);
    }
    puts("");

    // Setze globale Variablen
    OSMP_Size(&OSMP_size);
    OSMP_Rank(&OSMP_rank);

    free(shared_memory_name);
    log_to_file(2, "Free 256 B (shared_memory_name)");

    return OSMP_SUCCESS;
}

int OSMP_SizeOf(OSMP_Datatype datatype, unsigned int *size) {
    if(gettid() != getpid()){
        printf("Initializing of threads is not allowed\n");
        exit(0);
    }
    log_osmp_lib_call("OSMP_SizeOf");
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
    if(gettid() != getpid()){
        printf("Initializing of threads is not allowed\n");
        exit(0);
    }
    *size = shm_ptr->size;
    return OSMP_SUCCESS;
}

int OSMP_Rank(int *rank) {
    if(gettid() != getpid()){
        printf("Initializing of threads is not allowed\n");
        exit(0);
    }
    int pid = getpid();
    int size = shm_ptr->size;
    process_info* process;
    for(int i=0; i<size; i++) {
        // Iteriere durch alle Prozesse
        process = get_process_info(i);
        if (process->pid == pid) {
            *rank = process->rank;
            return OSMP_SUCCESS;
        }
    }
    return OSMP_FAILURE;
}


int OSMP_Send(const void *buf, int count, OSMP_Datatype datatype, int dest) {
    if(gettid() != getpid()){
        printf("Initializing of threads is not allowed\n");
        exit(0);
    }
    log_osmp_lib_call("OSMP_Send");
    if(count <= 0) {
        return OSMP_FAILURE;
    }
    unsigned int datatype_size;
    OSMP_SizeOf(datatype, &datatype_size);
    int length_in_bytes = (int)datatype_size * count;
    process_info * process_info = get_process_info(dest);
    semwait(&shm_ptr->mutex_shm_free_slots);
    sem_wait(&process_info->postbox.sem_proc_empty);
    sem_wait(&shm_ptr->sem_shm_free_slots);
    int index;
    int get_value_result = sem_getvalue(&shm_ptr->sem_shm_free_slots, &index);
    if(get_value_result!=0){
        log_to_file(3, "Fail of sem_getvalue in OSMP_Send\n");
        exit(OSMP_FAILURE);
    }
    semsignal(&shm_ptr->mutex_shm_free_slots);
    mempcpy(&shm_ptr->slots[index].payload, buf, (unsigned int)length_in_bytes);
    shm_ptr->slots[index].len = length_in_bytes;
    semwait(&process_info->postbox.mutex_proc_in);
    int process_in_index = process_info->postbox.in_index;
    process_info->postbox.postbox[process_in_index] = index;
    ++process_in_index;
    if (process_in_index==OSMP_MAX_MESSAGES_PROC){
        process_in_index=0;
    }
    process_info->postbox.in_index = process_in_index;
    semsignal(&process_info->postbox.mutex_proc_in);
    sem_post(&process_info->postbox.sem_proc_full);
    return OSMP_SUCCESS;
}

int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len) {
    if(gettid() != getpid()){
        printf("Initializing of threads is not allowed\n");
        exit(0);
    }
    log_osmp_lib_call("OSMP_Recv");
    if(count <= 0) {
        return OSMP_FAILURE;
    }
    unsigned int datatype_size;
    int length_in_bytes, rank;
    OSMP_SizeOf(datatype, &datatype_size);
    length_in_bytes = (int)datatype_size * count;
    OSMP_Rank(&rank);
    int message_offset;
    message_slot* message_slot = get_next_message_slot(OSMP_rank, &message_offset);
    int max_to_copy = length_in_bytes < message_slot->len ? length_in_bytes : message_slot->len;
    memcpy(buf, message_slot->payload, (unsigned long) max_to_copy);
    *source = message_slot->from;
    *len = message_slot->len;
    memset(shm_ptr->slots[message_offset].payload, '\0', OSMP_MAX_PAYLOAD_LENGTH);
    semwait(&shm_ptr->mutex_shm_free_slots);
    int free_slot_index;
    int free_slot_index_result = sem_getvalue(&shm_ptr->sem_shm_free_slots, &free_slot_index);
    if(free_slot_index_result<0){
        log_to_file(3, "Fail of sem_getvalue in OSMP_Recv\n");
        exit(OSMP_FAILURE);
    }
    shm_ptr->free_slots[free_slot_index] = message_offset;
    semsignal(&shm_ptr->mutex_shm_free_slots);
    sem_post(&shm_ptr->sem_shm_free_slots);
    *source = message_slot->from;
    return OSMP_SUCCESS;
}

int OSMP_Finalize(void) {
    log_osmp_lib_call("OSMP_Finalize");
    int result = close(shared_memory_fd);
    if(result==-1){
        log_to_file(3, "Couldn't close shared memory FD.");
        return OSMP_FAILURE;
    }
    result = munmap(shm_ptr, (size_t)memory_size);
    if(result==-1){
        log_to_file(3, "Couldn't unmap memory.");
        return OSMP_FAILURE;
    }
    logging_close();
    return OSMP_SUCCESS;
}

int OSMP_Barrier(void) {
    log_osmp_lib_call("OSMP_Barrier");
    return barrier_wait(&(shm_ptr->barrier));
}   

int OSMP_Gather(void *sendbuf, int sendcount, OSMP_Datatype sendtype, void *recvbuf, int recvcount, OSMP_Datatype recvtype, int recv) {
    --recv;
    log_osmp_lib_call("OSMP_Gather");
    if(gettid() != getpid()){
        printf("Initializing of threads is not allowed\n");
        exit(0);
    }
    shm_ptr->gather_t.flag = NOT_SAVED;
    unsigned int datatype_size;
    unsigned int length_in_bytes;
    OSMP_SizeOf(sendtype, &datatype_size);
    length_in_bytes = datatype_size * (unsigned int) sendcount;
    OSMP_Barrier();
    process_info * process = get_process_info(OSMP_rank);
    mempcpy(&process->gather_slot.payload, sendbuf, length_in_bytes);
    OSMP_Barrier();
    semwait(&shm_ptr->gather_t.mutex);
    while (shm_ptr->gather_t.flag == NOT_SAVED){
        if(OSMP_rank!=recv){
            pthread_cond_wait(&shm_ptr->gather_t.condition_variable, &shm_ptr->gather_t.mutex);
        }else{
            OSMP_SizeOf(recvtype, &datatype_size);
            length_in_bytes = datatype_size * (unsigned int) recvcount;
            for (int i = 0; i < OSMP_rank; ++i) {
                process_info * process_to_read_from = get_process_info(i);
                char * temp = recvbuf;
                temp += (unsigned int) i * length_in_bytes;

                mempcpy(temp, &process_to_read_from->gather_slot.payload, length_in_bytes);
            }
            shm_ptr->gather_t.flag = SAVED;
            pthread_cond_broadcast(&(shm_ptr->gather_t.condition_variable));
        }
    }
    return OSMP_SUCCESS;
}

int OSMP_ISend(const void *buf, int count, OSMP_Datatype datatype, int dest, OSMP_Request request) {
    if(gettid() != getpid()){
        printf("Initializing of threads is not allowed\n");
        exit(0);
    }
    log_osmp_lib_call("OSMP_ISend");
    puts("OSMP_ISend() not implemented yet");
    UNUSED(buf);
    UNUSED(count);
    UNUSED(datatype);
    UNUSED(dest);
    UNUSED(request);
    return OSMP_FAILURE;
}

int OSMP_IRecv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len, OSMP_Request request) {
    if(gettid() != getpid()){
        printf("Initializing of threads is not allowed\n");
        exit(0);
    }
    log_osmp_lib_call("OSMP_IRecv");
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
    if(gettid() != getpid()){
        printf("Initializing of threads is not allowed\n");
        exit(0);
    }
    log_osmp_lib_call("OSMP_Test");
    puts("OSMP_Test not implemented yet");
    UNUSED(request);
    UNUSED(flag);
    return OSMP_FAILURE;
}

int OSMP_Wait(OSMP_Request request) {
    if(gettid() != getpid()){
        printf("Initializing of threads is not allowed\n");
        exit(0);
    }
    log_osmp_lib_call("OSMP_Wait");
    puts("OSMP_Wait() not implemented yet");
    UNUSED(request);
    return OSMP_FAILURE;
}

int OSMP_CreateRequest(OSMP_Request *request) {
    if(gettid() != getpid()){
        printf("Initializing of threads is not allowed\n");
        exit(0);
    }
    log_osmp_lib_call("OSMP_CreateRequest");
    puts("OSMP_CreateRequest() not implemented yet");
    UNUSED(request);
    return OSMP_FAILURE;
}

int OSMP_RemoveRequest(OSMP_Request *request) {
    if(gettid() != getpid()){
        printf("Initializing of threads is not allowed\n");
        exit(0);
    }
    log_osmp_lib_call("OSMP_RemoveRequest");
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

void OSMP_GetSharedMemoryPointer(char **shared_memory) {
    *shared_memory = (char*)shm_ptr;
}
