#ifndef BETRIEBSSYSTEME_OSMPLIB_H
#define BETRIEBSSYSTEME_OSMPLIB_H
#include <stdio.h>

#include "OSMP.h"

/**
 * Dieses Makro wird verwendet, um den Compiler davon zu überzeugen, dass eine Variable verwendet wird.
 */
#define UNUSED(x) { (void)(x); }

/**
 * Mit diesem Wert wird signalisiert, dass ein Prozess keine weitere Nachricht hat.
 */
#define NO_MESSAGE 0

/**
 * Mit diesem Wert werden alle nicht genutzten Elemente des Arrays belegt, in dem die freien Nachrichtenslots verzeichnet sind.
 */
#define NO_SLOT 0

/**
 * Flag für einen freien Nachrichtenslot
 */
#define SLOT_FREE 1

/**
 * Flag für einen belegten Nachrichtenslot
 */
#define SLOT_TAKEN 0

/**
 * Struct für eine Nachricht entsprechend der Definition unseres Shared Memory.
 */
typedef struct OSMP_message {
    int free;                               // Flag: Slot frei (SLOT_FREE) oder nicht (SLOT_TAKEN)?
    int to;                                 // Rang des empfangenden Prozesses
    int from;                               // Rang des sendenden Prozesses
    int len;                                // Länge der Nachricht in Bytes
    OSMP_Datatype type;                     // Datentyp der enthaltenen Nachricht
    char payload[OSMP_MAX_PAYLOAD_LENGTH];  // eigentliche Nachricht
    int next_message;                       // Offset (rel. zum Anfang des SHM) zur nächsten Nachricht des Empfängers (NO_MESSAGE = keine weitere Nachricht)
} OSMP_message;

int get_free_slots_list_offset();

int get_postboxes_offset();

int calculate_shared_memory_size(int processes);

void OSMP_Init_Runner(int fd, char* shm, int size);

#endif //BETRIEBSSYSTEME_OSMPLIB_H
