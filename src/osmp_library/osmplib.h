#ifndef BETRIEBSSYSTEME_OSMPLIB_H
#define BETRIEBSSYSTEME_OSMPLIB_H
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

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
 * Flag, um eine korrekt initialisierte Barrier zu kennzeichnen.
 */
#define BARRIER_VALID 1

/**
 * Flag, um singlanisieren, dass der Reciever nicht alle Nachrichten in Gather gespeichert hat.
 */
#define NOT_SAVED 1

/**
 * Flag, um singlanisieren, dass der Reciever alle Nachrichten in Gather gespeichert hat.
 */
#define SAVED 0

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
} message_slot;

typedef struct {
    /**
     * @var postbox
     * Array, das alle Nachrichten für den Prozess enthält (als Index des Nachrichtenslots).
     */
    int postbox[OSMP_MAX_MESSAGES_PROC];

    /**
     * @var in_index
     * Index, der auf den nächsten freien Platz im Postfach zeigt.
     */
    int in_index;

    /**
     * @var mutex_proc_in
     * Mutex für die Synchronisierung der in_index-Variable.
     */
    pthread_mutex_t mutex_proc_in;

    /**
     * @var out_index
     * Index, der auf die nächste Nachricht im Postfach zeigt.
     */
    int out_index;

    /**
     * @var mutex_proc_out
     * Mutex für die Synchronisierung der out_index-Variable.
     */
    pthread_mutex_t mutex_proc_out;

    /**
     * @var sem_proc_empty
     * Semaphore für freie Plätze im Postfach.
     */
    sem_t sem_proc_empty;

    /**
     * @var sem_proc_full
     * Semaphore für belegte Plätze im Postfach.
     */
    sem_t sem_proc_full;
} postbox_utilities;

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
     * Postfach des Prozesses.
     */
    postbox_utilities postbox;

    /**
     * @var gather_slot
     * Der Gather-Slot des Prozesses.
     */
    message_slot gather_slot;
} process_info;

/* Datentyp zur Beschreibung einer Barriere. */
typedef struct barrier_t {
    pthread_mutex_t mutex; /* Zugriffskontrolle */
    pthread_cond_t convar; /* Warten auf Barriere */
    int valid; /* gesetzt, wenn Barriere initalisiert */
    int counter; /* Threads zaehlen */
    int cycle; /* Flag ob Barriere aktiv ist */
} barrier_t;

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
     * @var sem_shm_free_slots;
     * Semaphore für die Vergabe von Nachrichtenslots.
     */
    sem_t sem_shm_free_slots;

    /**
     * @var mutex_shm_free_slots
     * Mutex zur Synchronisierung des Zugriffs auf die Liste der freien Nachrichtenslots.
     */
    pthread_mutex_t mutex_shm_free_slots;

    /**
     * @var slots
     * Array mit allen 1:1-Nachrichtenslots.
     */
    message_slot slots[OSMP_MAX_SLOTS];

    /**
     * @var gather_mutex
     * Mutex für den Zugriff auf alle Gather-Slots durch ein und denselben Prozess (lesender Gather-Root-Prozess).
     */
    pthread_mutex_t gather_mutex;

    /**
     * @var barrier
     * Barriere.
     */
    barrier_t barrier;

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

process_info* get_process_info(int rank);

#endif //BETRIEBSSYSTEME_OSMPLIB_H
