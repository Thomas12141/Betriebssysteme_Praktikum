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
#include <stdlib.h>

shared_memory *shm_ptr = NULL;
int shared_memory_fd, OSMP_size, OSMP_rank = OSMP_FAILURE, memory_size;
thread_node * erster_thread = NULL;
thread_node * letzter_thread = NULL;

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
    pthread_mutex_lock(&shm_ptr->mutex_shm_free_slots);
    //Den index für den Array.
    sem_getvalue(&shm_ptr->sem_shm_free_slots, &index);
    //Runterzählen
    sem_post(&shm_ptr->sem_shm_free_slots);
    slot = shm_ptr->free_slots[index];
    pthread_mutex_unlock(&shm_ptr->mutex_shm_free_slots);

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
 * Gibt den Index des Nachrichtenslots zurück, in dem die nächste Nachricht für den aufrufenden Prozess liegt.
 * @return Index des Slots, in dem die nächste Nachricht für den aufrufenden Prozess liegt.
 */
int get_next_message(void ) {
    log_osmp_lib_call("get_next_message");
    process_info* process = get_process_info(OSMP_rank);

    sem_wait(&process->postbox.sem_proc_full);

    pthread_mutex_lock(&process->postbox.mutex_proc_out);

    int out_index = process->postbox.out_index;
    int slot_index = process->postbox.postbox[out_index];
    process->postbox.postbox[out_index] = NO_MESSAGE;
    --(process->postbox.out_index);
    if(process->postbox.out_index<0){
        process->postbox.out_index= OSMP_MAX_MESSAGES_PROC-1;
    }

    pthread_mutex_unlock(&process->postbox.mutex_proc_out);

    sem_post(&process->postbox.sem_proc_empty);

    return slot_index;
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
    // Thread-Safety:
    if(getpid()!=gettid()) {
        return OSMP_FAILURE;
    }

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

    (barrier->counter)--;

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

    status = pthread_mutex_unlock(&(barrier->mutex));
    if(status != 0) {
        log_to_file(3, "Failed to unlock barrier mutex!");
        return OSMP_FAILURE;
    }
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

    int pid = getpid();
    int size = shm_ptr->size;
    process_info* process;
    for(int i=0; i<size; i++) {
        // Iteriere durch alle Prozesse
        process = get_process_info(i);
        if (process->pid == pid) {
            OSMP_rank = process->rank;
            process->available = AVAILABLE;
        }
    }

    if(OSMP_rank == OSMP_FAILURE){
        log_to_file(3, "Couldn't find rank in the shared memory.\n");
        return OSMP_FAILURE;
    }

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
        return OSMP_FAILURE;
    }
    if(OSMP_rank == OSMP_FAILURE){
        log_to_file(3, "Calling rank before initializing the OSMP_Process.\n");
    }
    *rank = OSMP_rank;
    return OSMP_SUCCESS;
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
    if(dest>=OSMP_size || dest<0){
        return OSMP_FAILURE;
    }
    unsigned int datatype_size;
    OSMP_SizeOf(datatype, &datatype_size);
    int length_in_bytes = (int)datatype_size * count;
    if(length_in_bytes > OSMP_MAX_PAYLOAD_LENGTH) {
        return OSMP_FAILURE;
    }
    process_info * process_info = get_process_info(dest);

    if(process_info->available == NOT_AVAILABLE){
        log_to_file(2, "Trying to write to a not available process.\n");
        return OSMP_FAILURE;
    }
    sem_wait(&process_info->postbox.sem_proc_empty);
    sem_wait(&shm_ptr->sem_shm_free_slots);

    pthread_mutex_lock(&shm_ptr->mutex_shm_free_slots);

    // aktueller Index für das free-slots-Array
    int list_index = shm_ptr->free_slots_index;
    // hole Index eines freien Nachrichtenslots
    int slot_index = shm_ptr->free_slots[list_index];
    // setze Array an der Stelle auf NO_SLOT
    shm_ptr->free_slots[list_index] = NO_SLOT;
    // inkrementiere Index
    (shm_ptr->free_slots_index)++;

    pthread_mutex_unlock(&shm_ptr->mutex_shm_free_slots);

    // Schreibe Nachricht in Slot
    mempcpy(&shm_ptr->slots[slot_index].payload, buf, (unsigned int)length_in_bytes);
    shm_ptr->slots[slot_index].len = length_in_bytes;

    pthread_mutex_lock(&process_info->postbox.mutex_proc_in);

    int process_in_index = process_info->postbox.in_index;
    process_info->postbox.postbox[process_in_index] = slot_index;
    ++process_in_index;
    if (process_in_index==OSMP_MAX_MESSAGES_PROC){
        process_in_index=0;
    }
    process_info->postbox.in_index = process_in_index;

    pthread_mutex_unlock(&process_info->postbox.mutex_proc_in);

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
    OSMP_SizeOf(datatype, &datatype_size);
    int length_in_bytes = (int)datatype_size * count;

    int slot_index = get_next_message();
    message_slot* slot = &(shm_ptr->slots[slot_index]);

    if(length_in_bytes < slot->len) {
        log_to_file(3, "Recv buffer too small!");
    }
    memcpy(buf, slot->payload, (unsigned long) length_in_bytes);
    *source = slot->from;
    *len = slot->len;
    memset(shm_ptr->slots[slot_index].payload, '\0', OSMP_MAX_PAYLOAD_LENGTH);

    pthread_mutex_lock(&shm_ptr->mutex_shm_free_slots);

    // Lies aktuellen Index in der Liste freier Slots
    int list_index = shm_ptr->free_slots_index;
    // Füge eben geleertes Postfach zur Liste hinzu
    shm_ptr->free_slots[list_index-1] = slot_index;
    // Passe Listenindex an
    (shm_ptr->free_slots_index)--;

    pthread_mutex_unlock(&shm_ptr->mutex_shm_free_slots);

    sem_post(&shm_ptr->sem_shm_free_slots);
    return OSMP_SUCCESS;
}

int OSMP_Finalize(void) {
    log_osmp_lib_call("OSMP_Finalize");
    int result, semval;

    process_info* info = get_process_info(OSMP_rank);

    // Ein Flag, damit es bewusst wird, dass der Prozess nicht erreichbar ist.
    info->available = NOT_AVAILABLE;

    //TODO: Kein getValue mehr.
    // Prüfe, ob noch Nachrichten vorhanden sind
    result = sem_getvalue(&(info->postbox.sem_proc_full), &semval);
    if(result != 0) {
        log_to_file(3, "Call to sem_getvalue() from OSMP_Finalize failed");
        return OSMP_FAILURE;
    }

    if(semval > 0) {
        // lies alle restlichen Nachrichten
        char* buffer[OSMP_MAX_PAYLOAD_LENGTH];
        int source, len;
        for(int i=0; i<semval; i++) {
            result = OSMP_Recv(buffer, OSMP_MAX_PAYLOAD_LENGTH, OSMP_BYTE, &source, &len);
            if(result != OSMP_SUCCESS) {
                log_to_file(3, "Call to OSMP_Recv from OSMP_Finalize failed");
                return result;
            }
        }
    }

    result = close(shared_memory_fd);
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
// TODO: Doku Payload-Größe
int OSMP_Gather(void *sendbuf, int sendcount, OSMP_Datatype sendtype, void *recvbuf, int recvcount, OSMP_Datatype recvtype, int root) {
    // TODO:
    // TODO:
    log_osmp_lib_call("OSMP_Gather");
    int rank, rv;
    rv = OSMP_Rank(&rank);
    if(rv != OSMP_SUCCESS){
        log_to_file(3, "Initializing of threads is not allowed for Gather\n");
        return OSMP_FAILURE;
    }
    unsigned int send_datatype_size, receive_datatype_size;
    unsigned int send_length_in_bytes, receive_length_in_bytes;
    OSMP_SizeOf(sendtype, &send_datatype_size);
    send_length_in_bytes = send_datatype_size * (unsigned int) sendcount;
    if(send_length_in_bytes > OSMP_MAX_PAYLOAD_LENGTH){
        log_to_file(3, "Trying to send more bytes than the payload size.\n");
        return OSMP_FAILURE;
    }
    process_info * process = get_process_info(OSMP_rank);
    OSMP_SizeOf(recvtype, &receive_datatype_size);
    receive_length_in_bytes = (unsigned int) recvcount * receive_datatype_size;
    if(receive_length_in_bytes != send_length_in_bytes * (unsigned int) OSMP_size){
        log_to_file(3, "The size of the receiving buffer isn't the same, as the writing size.\n");
        return OSMP_FAILURE;
    }
    // Kopiere zu sendene Nachricht in den eigenen Gather-Slot
    mempcpy(&(process->gather_slot.payload), sendbuf, send_length_in_bytes);
    process->gather_slot.len = (int) send_length_in_bytes;
    // Warte, bis alle Prozesse geschrieben haben
    OSMP_Barrier();

    // Nur der Root-Prozess (empfangender Prozess) sammelt alle Nachrichten
    // TODO: Rank statt recv (vgl. geänderte Doku)
    if(rank == root) {
        int max_bytes = (int)(send_datatype_size) * recvcount;

        pthread_mutex_lock(&shm_ptr->gather_mutex);
        OSMP_SizeOf(recvtype, &send_datatype_size);
        char * temp = recvbuf;
        int written = 0;

        // TODO: beachten, wenn recv-datatype von send-datatype abweicht
        for (int i = 0; i < OSMP_size; ++i) {
            process_info * process_to_read_from = get_process_info(i);
            int to_copy = process_to_read_from->gather_slot.len;
            if(to_copy > (max_bytes - written)) {
                // recv-Buffer ist nicht groß genug für die folgende Nachricht
                break;
            }
            mempcpy(temp, &(process_to_read_from->gather_slot.payload),
                    (unsigned long) to_copy);
            temp += to_copy;
            written += to_copy;
        }
        
        pthread_mutex_unlock(&shm_ptr->gather_mutex);
    }
    // Warte, bis Root gelesen hat
    OSMP_Barrier();
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
