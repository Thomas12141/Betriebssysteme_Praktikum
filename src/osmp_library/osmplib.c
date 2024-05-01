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
 * Gibt den Offset (relativ zum Beginn des Shared Memory) zurück, an dem das Postfach des Prozesses mit dem angegebenen
 * Rang liegt.
 * @param rank Der Rang des Prozesses, dem das Postfach gehört.
 * @return Offset (relativ zum Beginn des Shared Memory), an dem das Postfach des aufrufenden Prozesses liegt.
 */
int get_postbox_offset(int rank) {
    log_osmp_lib_call(__TIMESTAMP__, "get_postbox_offset");
    unsigned int int_size;
    OSMP_SizeOf(OSMP_INT, &int_size);
    return get_postboxes_offset() + rank * (int)int_size;
}

/**
 * Gibt den nächsten freien Nachrichtenslots zurück. Dieser wird aus der Liste der freien Slots gelöscht, alle übrigen
 * Einträge im Array rücken eine Stelle nach vorne.
 * Das Flag im Slot selbst wird dadurch noch nicht auf "belegt" gesetzt! Dafür ist der Aufrufende verantwortlich.
 * @return Der Offset (relativ zum Beginn des SHM) des nächsten freien Nachrichtenslots. NO_SLOT, wenn kein Slot frei ist.
 */
int get_next_free_slot() {
    log_osmp_lib_call(__TIMESTAMP__, "get_next_free_slot");
    int slot;
    char* slot_list;
    unsigned int int_size;
    OSMP_SizeOf(OSMP_INT, &int_size);
    slot_list = shm_ptr + get_postboxes_offset();
    memcpy(&slot, shm_ptr + get_postboxes_offset(), int_size);

    // alle Elemente eine Stelle nach vorne rücken
    memmove(slot_list, slot_list+int_size, ((unsigned int) (OSMP_size - 1)) * int_size);
    // letzte Stelle auf NO_SLOT setzen
    int no_slots = NO_SLOT;
    char* last_array_element = slot_list + ((unsigned int) (OSMP_size - 1)) * int_size;
    memcpy(last_array_element, &no_slots, int_size);

    return slot;
}

/**
 * Gibt den Offset zu dem Slot zurück, in dem die nächste Nachricht für den aufrufenden Prozess liegt.
 * @return Offset in Bytes zu dem Slot, in dem die nächste Nachricht für den Prozess liegt. NO_MESSAGE, wenn das Postfach leer ist.
 */
int get_next_message_slot(void) {
    log_osmp_lib_call(__TIMESTAMP__, "get_next_message_slot");
    int slot, offset;
    unsigned int int_size;
    OSMP_SizeOf(OSMP_INT, &int_size);
    // Offset zum Postfach des aufrufenden Prozesses
    offset = get_postbox_offset(OSMP_rank);
    memcpy(&slot, shm_ptr+offset+OSMP_rank, int_size);
    return slot;
}

/**
 * Prüft das Postfach des Prozesses mit dem angegeben Rang auf die Anzahl der für ihn bereiten Nachrichten.
 * @param rank Der Rang des Prozesses, dessen Postfach überprüft werden soll.
 * @return Die Anzahl der Nachrichten, die für den aufrufenden Prozess bereit sind.
 */
int get_number_of_messages(int rank) {
    log_osmp_lib_call(__TIMESTAMP__, "get_number_of_messages");
    int number = 0, offset, message_slot;
    unsigned int int_size;
    // Offset zum Postfach des fraglichen Prozesses
    offset = get_postbox_offset(rank);
    OSMP_SizeOf(OSMP_INT, &int_size);
    // Iteriere durch alle Nachrichten des Prozesses
    memcpy(&message_slot, shm_ptr+offset, int_size);
    while(message_slot != NO_MESSAGE) {
        number++;
        OSMP_message* message = (OSMP_message*)shm_ptr + message_slot;
        message_slot = message->next_message;
    }
    return number;
}

/**
 * Gibt den Offset (relativ zum Beginn des SHM) zurück, an dem die letzte Nachricht für einen Prozess liegt.
 * @param rank Der Rang des Prozesses, dessen letzter Nachrichtenslot gesucht wird.
 * @return Der Offset (relativ zum Beginn des SHM), an dem die letzte Nachricht für den Prozess mit Rang rank liegt. NO_MESSAGE, wenn keine Nachricht für den Prozess bereitliegt.
 */
int get_last_message_slot(int rank) {
    log_osmp_lib_call(__TIMESTAMP__, "get_last_message_slot");
    int offset, message_slot, old_message_slot;
    unsigned int int_size;
    // Offset zum Postfach des fraglichen Prozesses
    offset = get_postbox_offset(rank);
    OSMP_SizeOf(OSMP_INT, &int_size);
    // Betrachte Postfach des Prozesses
    memcpy(&message_slot, shm_ptr+offset, int_size);
    old_message_slot = message_slot;
    // Wenn weitere Nachricht vorhanden ist, prüfe alle weiteren Slots bis zum letzten
    while(message_slot != NO_MESSAGE) {
        OSMP_message* message = (OSMP_message*)shm_ptr + message_slot;
        old_message_slot = message_slot;
        message_slot = message->next_message;
    }
    return old_message_slot;
}

/**
 * Verweist in der letzten Nachricht auf die neue Nachricht. Wenn der Empfänger aktuell keine anderen Nachrichten hat,
 * wird in seinem Postfach auf die neue Nachricht verwiesen.
 * @param dest Rang des empfangenden Prozesses
 * @param new_message_slot Offset (rel. zum Beginn des SHM) der neuen Nachricht, auf die verwiesen werden soll
 */
void reference_new_message(int dest, int new_message_slot) {
    // Erhalte letzte Nachricht des empfangenden Prozesses
    int last_message_slot = get_last_message_slot(dest);
    if (last_message_slot == NO_MESSAGE) {
        // vorher noch keine Nachricht vorhanden => schreibe in Postfach
        int postbox = get_postbox_offset(dest);
        char* addr = shm_ptr + postbox;
        unsigned int int_size;
        OSMP_SizeOf(OSMP_INT, &int_size);
        memcpy(addr, &new_message_slot, int_size);
    }
    else {
        // vorher schon Nachricht(en) vorhanden => schreibe ans Ende der letzten Nachricht
        OSMP_message* last_message = (OSMP_message*)(shm_ptr+last_message_slot);
        last_message->next_message = new_message_slot;
    }
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
        printf("Failed to open shared memory.\n");
        return OSMP_FAILURE;
    }
    shm_ptr = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    logging_init_child(shm_ptr);
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
    for (int i = 0; i < osmp_size; ++i) {
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
    if(count <= 0) {
        return OSMP_FAILURE;
    }
    // TODO: Synchronisierung
    unsigned int datatype_size;
    OSMP_SizeOf(datatype, &datatype_size);
    int length_in_bytes = (int)datatype_size * count;
    int buf_offset = 0;
    do {
        // warten, bis ein Nachrichtenslot für den empfangenden Prozess frei ist
        while (get_number_of_messages(dest) >= get_OSMP_MAX_MESSAGES_PROC()) {
            // TODO: blockieren;
        }
        // Erhalte den Offset (relativ zum Beginn des SHM) zum nächsten freien Slot
        int slot = get_next_free_slot();
        // Berechne die Adresse dieses Slots
        char* slot_addr = shm_ptr + slot;
        // zu kopierende Bytes = min(OSMP_MAX_PAYLOAD_LENGTH, length_in_bytes)
        int to_copy = OSMP_MAX_PAYLOAD_LENGTH < length_in_bytes ? OSMP_MAX_PAYLOAD_LENGTH : length_in_bytes;
        OSMP_message* message = (OSMP_message*) slot_addr;
        message->free = SLOT_TAKEN;
        message->type = datatype;
        memcpy(message->payload, (char*)buf + buf_offset, (unsigned long) to_copy);
        message->next_message = NO_MESSAGE;
        // Verweise in der letzten Nachricht auf diese neue Nachricht
        reference_new_message(dest, slot);
        // Aktualisiere Variablen für nächsten Schleifendurchlauf
        length_in_bytes -= to_copy;
        buf_offset += to_copy;
    } while(length_in_bytes > 0);
    return OSMP_SUCCESS;
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
    logging_close();
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

void OSMP_GetSharedMemoryPointer(char **shared_memory) {
    *shared_memory = shm_ptr;
}
