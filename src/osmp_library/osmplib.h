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
 * Flag, um singlanisieren, dass der Prozess nicht erreichbar ist.
 */
#define NOT_AVAILABLE 1

/**
 * Flag, um singlanisieren, dass der Prozess erreichbar ist.
 */
#define AVAILABLE 0

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

    /**
     * @var sem_proc_full_counter
     * A number representing the value of the semaphore.
     */
     int sem_proc_full_value;
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

    /**
     * @var available
     * Ein Flag um zu wiesen, ob dieser Prozess verfügbar ist.
     */
    int available;
} process_info;

/**
 * @struct barrier_t
 * @brief Datentyp zur Beschreibung einer Barriere. */
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
     * @var free_slots_index
     * Zeigt auf die Stelle in free_slots, an der das nächste freie Postfach liegt.
     */
    int free_slots_index;

    /**
     * @var sem_shm_free_slots;
     * Semaphore für die Vergabe von Nachrichtenslots.
     */
    sem_t sem_shm_free_slots;

    /**
     * @var mutex_shm_free_slots
     * Mutex zur Synchronisierung des Zugriffs auf die Liste der freien Nachrichtenslots und deren Index.
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
 * @struct thread_node
 * @brief Eine two way linked list von threads,
 */
typedef struct thread_node{

    /**
     * @var thread
     * @brief Der Thread in diesem Knoten.
     */
    pthread_t thread;

    /**
     * @var next
     * @brief Ein Pointer auf dem nächsten Knoten in der Liste, wenn es der letzte Element ist, dann zeigt es auf Null.
     */
    struct thread_node * next;

    /**
     * @var next
     * @brief Ein Pointer auf dem letzten Knoten in der Liste, wenn es der erste Element ist, dann zeigt es auf Null.
     */
    struct thread_node * prev;
} thread_node;

/**
 * @struct IParams
 * @brief Struct, das die ISend-/IRecv-Funktionsparameter speichert,
 */
typedef struct IParams {
    /**
     * @var mutex
     * Mutex für den Zugriff auf die Elemente des Structs.
     */
    pthread_mutex_t mutex;

    /**
     * @var convar
     * Condition-Variable, um auf das Fertigstellen des mit diesem Struct assoziierten Prozesses zu warten.
     */
    pthread_cond_t convar;

    /**
     * @var recv_buf
     * Übergabeparameter für Recv, der hier zwischengespeichert wird.
     */
    void* recv_buf;

    /**
     * @var send_buf
     * Übergabeparameter für Recv, der hier zwischengespeichert wird.
     */
    const void* send_buf;

    /**
     * @var count
     * Übergabeparameter für Send/Recv, der hier zwischengespeichert wird.
     */
    int count;

    /**
     * @var datatype
     * Übergabeparameter für Send/Recv, der hier zwischengespeichert wird.
     */
    OSMP_Datatype datatype;

    /**
     * @var dest
     * Übergabeparameter für Send, der hier zwischengespeichert wird.
     */
    int dest;       // nur für ISend

    /**
     * @var source
     * Übergabeparameter für Recv, der hier zwischengespeichert wird.
     */
    int* source;    // nur für IRecv

    /**
     * @var len
     * Übergabeparameter für Recv, der hier zwischengespeichert wird.
     */
    int* len;       // nur für IRecv

    /**
     * @var done
     * Flag, das signalisiert, ob der mit diesem Struct assoziierte blockierende Vorgang abgeschlossen ist.
     * Steht auf *OSMP_DONE*, wenn abgeschlossen, andernfalls auf *OSMP_WAITING*.
     */
    int done;
} IParams;

int calculate_shared_memory_size(int processes);

void OSMP_Init_Runner(int fd, shared_memory* shm, int size);

process_info* get_process_info(int rank);

#endif //BETRIEBSSYSTEME_OSMPLIB_H
