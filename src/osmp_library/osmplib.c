/**
 * In dieser Quelltext-Datei sind Implementierungen der OSMP Bibliothek zu finden.
 */
#define SHARED_MEMORY_NAME "/shared_memory"

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

shared_memory *shm_ptr;
char * locks_shared_memory;
int shared_memory_fd, OSMP_size, OSMP_rank, memory_size;
pthread_mutex_t * OSMP_send_mutex;
pthread_cond_t * OSMP_send_condition;

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
 * Gibt die Nummer des nächsten freien Nachrichtenslots zurück. Dieser wird aus der Liste der freien Slots gelöscht,
 * alle übrigen Einträge im Array rücken eine Stelle nach vorne.
 * Das Flag im Slot selbst wird dadurch noch nicht auf "belegt" gesetzt! Dafür ist der Aufrufende verantwortlich.
 * @return Die Nummer des nächsten freien Nachrichtenslots. NO_SLOT, wenn kein Slot frei ist.
 */
int get_next_free_slot() {
    log_osmp_lib_call(__TIMESTAMP__, "get_next_free_slot");
    int slot;
    // verwende ersten freien Slot als Rückgabewert
    slot = shm_ptr->free_slots[0];
    // alle Elemente eine Stelle nach vorne rücken
    for(int i=0; i<(OSMP_MAX_SLOTS-1); i++) {
        shm_ptr->free_slots[i] = shm_ptr->free_slots[i+1];
    }
    // letzte Stelle auf NO_SLOT setzen
    shm_ptr->free_slots[OSMP_MAX_SLOTS-1] = NO_SLOT;

    return slot;
}

/**
 * Gibt einen Zeiger auf das process_info-Struct des angegebenen Prozesses zurück.
 * @param rank Rang des Prozesses, dessen process_info angefordert wird.
 * @return Zeiger auf process_info-Struct des Prozesses mit dem angegebenen Rang.
 */
process_info* get_process_info(int rank) {
    log_osmp_lib_call(__TIMESTAMP__, "get_process_info");
    // Prozess-Info 0
    process_info* info = &(shm_ptr->first_process_info);
    // Offset zum passenden Rang
    info += rank;
    return info;
}

/**
 * Gibt einen Zeiger auf den Nachrichtenslot zurück, in dem die nächste Nachricht für den angegebenen Prozess liegt.
 * @param rank Rang des Prozesses, dessen erster Nachrichtenslot zurückgegeben werden soll.
 * @return Zeiger auf den Slot, in dem die nächste Nachricht für den Prozess mit Rang *rank* liegt. NULL, wenn das Postfach leer ist.
 */
message_slot* get_next_message_slot(int rank) {
    log_osmp_lib_call(__TIMESTAMP__, "get_next_message_slot");
    process_info* process = get_process_info(rank);
    int slot_number = process->postbox;
    if(slot_number == NO_MESSAGE) {
        return NULL;
    }
    return &(shm_ptr->slots[slot_number]);
}

/**
 * Gibt die Nummer des Slots, in dem die nächste Nachricht für den angegebenen Prozess liegt.
 * @param rank Rang des Prozesses, dessen erster Nachrichtenslot zurückgegeben werden soll.
 * @return Die Nummer des Slots, in dem die nächste Nachricht für den Prozess mit Rang *rank* liegt. NO_MESSAGE, wenn das Postfach leer ist.
 */
int get_next_message_slot_number(int rank) {
    log_osmp_lib_call(__TIMESTAMP__, "get_next_message_slot");
    process_info* process = get_process_info(rank);
    return process->postbox;
}

/**
 * Prüft das Postfach des Prozesses mit dem angegeben Rang auf die Anzahl der für ihn bereiten Nachrichten.
 * @param rank Der Rang des Prozesses, dessen Postfach überprüft werden soll.
 * @return Die Anzahl der Nachrichten, die für den aufrufenden Prozess bereit sind.
 */
int get_number_of_messages(int rank) {
    log_osmp_lib_call(__TIMESTAMP__, "get_number_of_messages");

    int number = 0;
    int next_slot_number = get_next_message_slot_number(rank);
    while(next_slot_number != NO_MESSAGE) {
        number++;
        next_slot_number = shm_ptr->slots[next_slot_number].next_message;
    }
    return number;
}

/**
 * Gibt einen Zeiger auf den Nachrichtenslot, in dem die letzte Nachricht für einen Prozess liegt.
 * @param rank Der Rang des Prozesses, dessen letzter Nachrichtenslot gesucht wird.
 * @return Ein Zeiger auf den Nachrichtenslot, in dem die letzte Nachricht für den Prozess mit Rang rank liegt. NULL, wenn keine Nachricht für den Prozess bereitliegt.
 */
message_slot* get_last_message_slot(int rank) {
    log_osmp_lib_call(__TIMESTAMP__, "get_last_message_slot");

    int next_slot_number = get_next_message_slot_number(rank);
    message_slot* slot = NULL;
    while(next_slot_number != NO_MESSAGE) {
        slot = &(shm_ptr->slots[next_slot_number]);
        next_slot_number = slot->next_message;
    }
    return slot;
}

/**
 * Verweist in der letzten Nachricht auf die neue Nachricht. Wenn der Empfänger aktuell keine anderen Nachrichten hat,
 * wird in seinem Postfach auf die neue Nachricht verwiesen.
 * @param dest Rang des empfangenden Prozesses.
 * @param new_message_slot Nummer des Nachrichtenslots, auf den verwiesen werden soll.
 */
void reference_new_message(int dest, int new_message_slot_number) {
    log_osmp_lib_call(__TIMESTAMP__, "reference_new_message");

    // Erhalte Verweis auf letzte Nachricht des empfangenden Prozesses
    message_slot* last_message_slot = get_last_message_slot(dest);
    if (last_message_slot == NULL) {
        // vorher noch keine Nachricht vorhanden => schreibe in Postfach
        process_info* process = get_process_info(dest);
        process->postbox = new_message_slot_number;
    }
    else {
        // vorher schon Nachricht(en) vorhanden => schreibe ans Ende der letzten Nachricht
        last_message_slot->next_message = new_message_slot_number;
    }
}

/**
 * Setzt Flag im angegebenen Nachrichtenslot auf "frei", die Bytes im Nachrichtenpuffer auf 0 und den Verweis auf
 * die nächste Nachricht auf NO_MESSAGE.
 * @param slot_number Nummer des Nachrichtenslots, der geleert werden soll.
 */
void empty_message_slot(int slot_number) {
    log_osmp_lib_call(__TIMESTAMP__, "empty_message_slot");
    message_slot* slot = &(shm_ptr->slots[slot_number]);
    slot->free = SLOT_FREE;
    slot->to = NO_MESSAGE;
    slot->from = NO_MESSAGE;
    slot->len = 0;
    memset(slot->payload, '\0', OSMP_MAX_PAYLOAD_LENGTH);
    slot->next_message = NO_MESSAGE;
}

/**
 * Entfernt Nachricht aus der Nachrichtenkette und setzt die Referenzen davor und danach passend.
 * Der Nachrichtenslot wird außerdem mittels empty_message_slot() geleert.
 * @param slot_number Nummer des Nachrichtenslots, dessen Referenzen entfernt werden soll.
 */
int remove_message(int slot_number) {
    log_osmp_lib_call(__TIMESTAMP__, "remove_message");

    message_slot* message = &(shm_ptr->slots[slot_number]);
    // Finde Referenz auf diese Nachricht
    // Prüfe erste Nachricht im Postfach
    process_info* process = get_process_info(message->to);
    int postbox_message_number = process->postbox;
    if(postbox_message_number == slot_number) {
        // zu entfernende Nachricht war erste Nachricht im Postfach
        // ==> setze Postfach auf nächste Nachricht des zu leerenden Slots
        process->postbox = message->next_message;
        // Slot leeren und freigeben
        empty_message_slot(slot_number);
        return OSMP_SUCCESS;
    }
    // andernfalls durch verkettete Nachrichten iterieren, bis die passende erreicht wird
    int previous = postbox_message_number;
    int next = shm_ptr->slots[postbox_message_number].next_message;
    while(previous != slot_number && next != NO_MESSAGE) {
        previous = next;
        next = shm_ptr->slots[previous].next_message;
    }
    if(next == NO_MESSAGE) {
        // keinen Verweis auf angegebene Nachricht gefunden
        return OSMP_FAILURE;
    }
    // andernfalls: richtige Nachricht gefunden
    // setze Pointer der vorherigen Nachricht auf gewünschte Nachricht
    message_slot* previous_message = &(shm_ptr->slots[previous]);
    previous_message->next_message = message->next_message;
    // Slot leeren und freigeben
    empty_message_slot(slot_number);
    return OSMP_SUCCESS;
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
    char *shared_memory_name = calloc(MAX_PATH_LENGTH, sizeof(char));
    int locks_shared_memory_fd = shm_open("locks_shared_memory", O_RDWR, 0666);
    locks_shared_memory = mmap(NULL, sizeof(pthread_mutex_t) + sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED, locks_shared_memory_fd, 0);
    OSMP_send_mutex = (pthread_mutex_t*)locks_shared_memory;
    OSMP_send_condition = (pthread_cond_t *) OSMP_send_mutex + sizeof(pthread_mutex_t);
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

    log_osmp_lib_call(__TIMESTAMP__, "OSMP_Init");
    log_to_file(2, __TIMESTAMP__, "Calloc 256 B (shared_memory_name)");
    if(shm_ptr == MAP_FAILED){
        log_to_file(3, __TIMESTAMP__, "Failed to map shared memory.\n");
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
    log_to_file(2, __TIMESTAMP__, "Free 256 B (shared_memory_name)");

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
    memcpy(size, shm_ptr, sizeof(int));
    return OSMP_SUCCESS;
}

int OSMP_Rank(int *rank) {
    int pid = getpid();
    int size = shm_ptr->size;
    // Zeiger auf erste Prozess-Info am Ende des Structs
    process_info* first_process_info = &(shm_ptr->first_process_info);
    process_info* process;
    for(int i=0; i<size; i++) {
        // Offset über das Struct hinaus
        process = first_process_info + i;
        if (process->pid == pid) {
            *rank = process->rank;
            return OSMP_SUCCESS;
        }
    }
    return OSMP_FAILURE;
}


int OSMP_Send(const void *buf, int count, OSMP_Datatype datatype, int dest) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_Send");
    if(count <= 0) {
        return OSMP_FAILURE;
    }
    unsigned int datatype_size;
    OSMP_SizeOf(datatype, &datatype_size);
    int length_in_bytes = (int)datatype_size * count;
    int buf_offset = 0;
    do {
        pthread_mutex_lock(OSMP_send_mutex);
        // warten, bis ein Nachrichtenslot für den empfangenden Prozess frei ist
        while (get_number_of_messages(dest) >= get_OSMP_MAX_MESSAGES_PROC()) {
            pthread_cond_wait(OSMP_send_condition, OSMP_send_mutex);
        }
        // Erhalte Nummer des nächsten freien Slots
        int slot_number = get_next_free_slot();
        // Zeiger auf diesen Slot
        message_slot* slot = &(shm_ptr->slots[slot_number]);
        // zu kopierende Bytes = min(OSMP_MAX_PAYLOAD_LENGTH, length_in_bytes)
        int to_copy = OSMP_MAX_PAYLOAD_LENGTH < length_in_bytes ? OSMP_MAX_PAYLOAD_LENGTH : length_in_bytes;
        slot->free = SLOT_TAKEN;
        slot->to = dest;
        OSMP_Rank(&(slot->from));
        slot->len = to_copy;
        slot->type = datatype;
        memcpy(slot->payload, (char*)buf + buf_offset, (unsigned long) to_copy);
        pthread_mutex_unlock(OSMP_send_mutex);
        slot->next_message = NO_MESSAGE;
        // Verweise in der letzten Nachricht auf diese neue Nachricht
        reference_new_message(dest, slot->slot_number);
        // Aktualisiere Variablen für nächsten Schleifendurchlauf
        length_in_bytes -= to_copy;
        buf_offset += to_copy;
    } while(length_in_bytes > 0);
    return OSMP_SUCCESS;
}

int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_Recv");
    if(count <= 0) {
        return OSMP_FAILURE;
    }

    // TODO: Synchronisierung
    unsigned int datatype_size;
    int length_in_bytes, rank;
    OSMP_SizeOf(datatype, &datatype_size);
    length_in_bytes = (int)datatype_size * count;
    OSMP_Rank(&rank);
    message_slot* slot;
    do{
        slot = get_next_message_slot(rank);
    } while (slot == NULL);


    while(slot->type != datatype) {
        int next_slot = slot->next_message;
        if (next_slot == NO_MESSAGE) {
            do{
                next_slot = slot->next_message;
            } while (next_slot == NO_MESSAGE);
        }
        else {
            slot = &(shm_ptr->slots[next_slot]);
        }
    }
    // Nachricht gefunden --> kopiere sie in Buffer des Empfängers
    // zu kopierende Bytes = min(length_in_bytes, slot->len)
    int max_to_copy = length_in_bytes < slot->len ? length_in_bytes : slot->len;
    pthread_mutex_lock(OSMP_send_mutex);
    memcpy(buf, slot->payload, (unsigned long) max_to_copy);
    *source = slot->from;
    *len = slot->len;
    // Leere Slot und entferne Referenzen auf die Nachricht
    remove_message(slot->slot_number);
    pthread_mutex_unlock(OSMP_send_mutex);
    return OSMP_SUCCESS;
}

int OSMP_Finalize(void) {
    log_osmp_lib_call(__TIMESTAMP__, "OSMP_Finalize");
    int result = close(shared_memory_fd);
    if(result == -1){
        log_to_file(3, __TIMESTAMP__, "Couldn't close file descriptor memory.");
        return OSMP_FAILURE;
    }
    result = munmap(shm_ptr, (size_t)memory_size);
    if(result==-1){
        log_to_file(3, __TIMESTAMP__, "Couldn't unmap memory.");
        return OSMP_FAILURE;
    }
    logging_close();
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

void OSMP_GetSharedMemoryPointer(char **shared_memory) {
    *shared_memory = (char*)shm_ptr;
}
