#ifndef BETRIEBSSYSTEME_OSMPLIB_H
#define BETRIEBSSYSTEME_OSMPLIB_H
#include <stdio.h>
#include <pthread.h>

#include "OSMP.h"

/**
 * Dieses Makro wird verwendet, um den Compiler davon zu überzeugen, dass eine Variable verwendet wird.
 */
#define UNUSED(x) { (void)(x); }

/**
 * Mit diesem Wert wird signalisiert, dass ein Prozess keine weitere Nachricht hat.
 */
#define NO_MESSAGE (-1)

/**
 * Mit diesem Wert werden alle nicht genutzten Elemente des Arrays belegt, in dem die freien Nachrichtenslots
 * verzeichnet sind.
 */
#define NO_SLOT (-1)

/**
 * Flag für einen freien Nachrichtenslot.
 */
#define SLOT_FREE 1

/**
 * Flag für einen belegten Nachrichtenslot.
 */
#define SLOT_TAKEN 0

/**
 * Maximal erlaubte Länge des Pfads zur Logdatei, inkl. terminierendem Nullbyte.
 */
#define MAX_PATH_LENGTH 256

/**
 * @struct message_slot
 * @brief Struct für eine Nachricht entsprechend der Definition unseres Shared Memory.
 */
typedef struct message_slot {
    /**
     * @var slot_number
     * Nummer des Slots.
     */
    int slot_number;

    /**
     * @var free
     * Flag: Slot frei (SLOT_FREE) oder nicht (SLOT_TAKEN)?
     */
    int free;

    /**
     * @var to
     * Rang des empfangenden Prozesses.
     */
    int to;

    /**
     * @var from
     * Rang des sendenden Prozesses.
     */
    int from;

    /**
     * @var len
     * Länge der Nachricht in Bytes.
     */
    int len;

    /**
     * @var type
     * Datentyp der enthaltenen Nachricht
     */
    OSMP_Datatype type;

    /**
     * @var payload
     * Inhalt der Nachricht.
     */
    char payload[OSMP_MAX_PAYLOAD_LENGTH];

    /**
     * @var next_message
     * Nummer des slots, in dem die nächste Nachricht für den Empfänger liegt (NO_MESSAGE = keine weitere Nachricht).
     */
    int next_message;

    /**
     * @var slot_mutex
     * Mutex zur Synchronisierung des Zugriffs auf diesen Nachrichtenslot.
     */
    pthread_mutex_t slot_mutex;

    /**
     * @var slot_emptied
     * Condition-Variable, mit der signalisiert werden kann, dass ein Slot geleert wurde und somit eine neue Nachricht
     * aufnehmen kann.
     */
    pthread_cond_t slot_emptied;
} message_slot;

typedef struct{
    /**
     * @var mutex
     * Mutex für den Zugriff auf shared Variable.
     */
    pthread_mutex_t mutex;

    /**
     * @var condition_variable
     * Condition-Variable, um zu signalisieren, dass alle Prozesse, die an der Schlange sind, immer noch warten müssen.
     */
    pthread_cond_t condition_variable;

    /**
     * @var counter
     * Ein counter, der signalisiert, wer an der Reihe ist.
     */
    int counter;
} gather_struct;

/**
 * @struct process_info
 * @brief Struct für Informationen zu einem Prozess
 */
typedef struct process_info {
    /**
     * @var rank
     * Rang des Prozesses.
     */
    int rank;

    /**
     * @var pid
     * PID des Prozesses.
     */
    int pid;

    /**
     * @var postbox
     * Nummer des Slots, in dem die erste Nachricht für diesen Prozess liegt
     * (NO_MESSAGE = keine Nachricht für diesen Prozess).
     */
    int postbox;

    /**
     * @var postbox_mutex
     * Mutex zur Synchronisierung des Zugriffs auf das Postfach.
     */
    pthread_mutex_t postbox_mutex;

    /**
     * @var new_message
     * Condition-Variable, mit der signalisiert werden kann, dass eine neue Nachricht für den Prozess vorliegt,
     * der mit diesem Struct verwaltet wird.
     */
    pthread_cond_t new_message;
} process_info;

/**
 * @struct shared_memory
 * @brief Struct für den fixen Teil des Shared Memory gemäß unserer Spezifikation.
 */
typedef struct shared_memory {
    /**
     * @var size
     * Anzahl der Kindprozesse (entspricht OSMP_Size).
     */
    int size;

    /**
     * @var logging_mutex
     * Mutex für den Zugriff auf die Logdatei.
     */
    pthread_mutex_t logging_mutex;

    /**
     * @var free_slots
     * Liste der freien Nachrichtenslots.
     */
    int free_slots[OSMP_MAX_SLOTS];

    /**
     * @var free_slots_mutex
     * Mutex zur Synchronisierung des Zugriffs auf die Liste der freien Nachrichtenslots.
     */
    pthread_mutex_t free_slots_mutex;

    /**
     * @var slots
     * Array mit allen 1:1-Nachrichtenslots.
     */
    message_slot slots[OSMP_MAX_SLOTS];

    /**
     * @var gather_slot
     * Slot für eine einzelne Gather-Nachricht.
     */
    message_slot gather_slot;

    /**
     * @var barrier_mutex
     * Mutex für die Synchronisierung des Zugriffs auf den Barrier-Counter.
     */
     pthread_mutex_t barrier_mutex;

    /**
     * @var barrier_condition
     * Condition-Variable, um zu signalisieren, dass alle Prozesse an der Barriere warten.
     */
    pthread_cond_t barrier_condition;

    /**
     * @var barrier_counter
     * Counter für die Barrier-Funktion. Gibt die Anzahl der Prozesse zurück, die an der Barriere warten.
     */
    int barrier_counter;

    /**
     * @var logfile
     * Pfad zur Logdatei.
     */
    char logfile[MAX_PATH_LENGTH];

    /**
     * @var verbosity
     * Logging-Verbosität.
     */
    unsigned int verbosity;

    /**
     * @var gather_t
     * Der struct, um gather Funktion zu realisieren.
     */
    gather_struct gather_t;

    /**
     * @var process_info
     * Info zu Prozess 0. Speicher für weitere Prozess-Infos muss über die fixe Struct-Größe hinaus
     * dynamisch berechnet werden.
     */
    process_info first_process_info;
} shared_memory;

/**
 * @struct gather_struct
 * @brief struct für alle benötigte Teile, um gather zu realisieren.
 */

int calculate_shared_memory_size(int processes);

void OSMP_Init_Runner(int fd, shared_memory* shm, int size);

#endif //BETRIEBSSYSTEME_OSMPLIB_H
