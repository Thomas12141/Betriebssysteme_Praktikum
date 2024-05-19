#include "osmp_run.h"
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#include "../osmp_library/logger.h"
#include "../osmp_library/osmplib.h"

int shm_size;

/**
 * Shared memory name to be created following the scheme:
 * shared_memory_<runner_pid>
 */
char* shared_memory_name;

/**
 * Erzeugt einen Mutex mit dem Attribut "shared" und kopiert ihn an den gewünschten Speicherbereich.
 * @param mutex_pointer Zeiger auf den Speicherbereich, in den der neu erzeugte Mutex kopiert werden soll.
 * @return OSMP_SUCCESS im Erfolgsfall, sonst OSMP_FAILURE.
 */
int init_shared_mutex(pthread_mutex_t* mutex_pointer) {
    // Shared-Attribut für Mutex erzeugen
    pthread_mutexattr_t att;
    pthread_mutexattr_init(&att);
    pthread_mutexattr_setpshared(&att, PTHREAD_PROCESS_SHARED);
    // Mutex erstellen
    pthread_mutex_t* mutex = calloc(1, sizeof(pthread_mutex_t));
    // Mutex initialisieren
    int mutex_result = pthread_mutex_init(mutex, &att);
    if(mutex_result < 0){
        printf("Result of pthread_mutex_init = %d\n", mutex_result);
        return OSMP_FAILURE;
    }
    // Mutex in den angegebenen Bereich kopieren
    memcpy(mutex_pointer, mutex, sizeof(pthread_mutex_t));
    // lokale Variablen löschen/freigeben
    pthread_mutexattr_destroy(&att);
    pthread_mutex_destroy(mutex);
    free(mutex);
    return OSMP_SUCCESS;
}

/**
 * Erzeugt eine Condition-Variable mit dem Attribut "shared" und kopiert sie an den gewünschten Speicherbereich.
 * @param cond_pointer Zeiger auf den Speicherbereich, in den die neu erzeugte Condition-Variable kopiert werden soll.
 * @return OSMP_SUCCESS im Erfolgsfall, sonst OSMP_FAILURE.
 */
int init_shared_cond_var(pthread_cond_t* cond_pointer) {
    // Initialisiere Condition-Variable
    pthread_cond_t condition;
    // Attribut anlegen
    pthread_condattr_t condition_attr;
    // Attribut initialisieren und verändern
    int condition_result = pthread_condattr_init(&condition_attr);
    if(condition_result < 0){
        printf("Result of pthread_condattr_init = %d\n", condition_result);
        return OSMP_FAILURE;
    }
    pthread_condattr_setpshared(&condition_attr, PTHREAD_PROCESS_SHARED);
    condition_result = pthread_cond_init(&condition, &condition_attr);
    if(condition_result < 0){
        printf("Result of pthread_condr_init = %d\n", condition_result);
        return OSMP_FAILURE;
    }
    // Condition-Variable in Shared Memory kopieren
    memcpy(cond_pointer, &condition, sizeof(pthread_cond_t));
    pthread_cond_destroy(&condition);
    pthread_condattr_destroy(&condition_attr);
    return OSMP_SUCCESS;
}

int start_all_executables(int number_of_executables, char* executable, char ** arguments, shared_memory* shm_ptr){
    for (int i = 0; i < number_of_executables; ++i) {
        int pid = fork();
        for(int j = 0; arguments[j] != NULL; j++) {
            printf("%s ", arguments[j]);
        }
        puts("");
        if (pid < 0) {
            log_to_file(3,"failed to fork");
            return -1;
        } else if (pid == 0) {//Child process.
            execv(executable, arguments);
            log_to_file(3,"execv failed");
            return -1;
        } else{
            // Setze PID in Process Info im Shared Memory
            // Offset berechnen (alle außer der 0. Prozess-Info gehen über SHM-Struct hinaus)
            process_info* info = &(shm_ptr->first_process_info) + i;
            info->pid = pid;
        }
    }
    return 0;
}

int freeAll(int shm_fd, shared_memory* shm_ptr){
    int result = munmap(shm_ptr, (size_t) shm_size);
    if(result==-1){
        log_to_file(3, "Couldn't unmap memory.");
        return -1;
    }
    result = close(shm_fd);
    if(result==-1){
        log_to_file(3, "Couldn't close file descriptor memory.");
        return -1;
    }
    result = shm_unlink(shared_memory_name);
    if(result==-1){
        log_to_file(3, "Couldn't unlink file name.");
        return -1;
    }
    log_to_file(2, "Freeing shared_memory_name");
    free(shared_memory_name);
    return 0;
}

/**
 * Überprüft, ob ein String nur Leerzeichen enthält.
 *
 * @param string Zeiger auf den zu überprüfenden String (muss null-terminiert sein).
 *
 * @return 1, wenn der String nur Leerzeichen enthält; sonst 0.
 */
int is_whitespace(const char* string) {
    if(string == NULL) {
        return 0;
    }
    for(int i=0; string[i] != '\0'; i++) {
        if(string[i] == ' ') {
            // Leerzeichen => prüfe nächstes Zeichen
            continue;
        }
        // kein Leerzeichen => String enthält nicht nur Leerzeichen
        return 0;
    }
    return 1;
}

/**
 * Gibt die korrekte Verwendung des Programms aus.
 */
void printUsage(void) {
    printf("Usage: ./osmp_run <ProcAnzahl> [-L <PfadZurLogDatei> [-V <LogVerbosität>]] ./<osmp_executable> [<param1> <param2> ...]\n");
}

/**
 * Gibt einen Hinweis zur maximalen Pfadlänge der Logdatei aus.
 */
void print_logfile_condition(void) {
    printf("Der Pfad zur Logdatei darf nicht länger als %d Zeichen (inkl. Nullbyte) sein.\n", MAX_PATH_LENGTH);
}

/**
 * Diese Funktion analysiert und parst die Befehlszeilenargumente. Wenn die Argumente nicht dem geforderten Schema
 * ./osmp_run <ProcAnzahl> [-L <PfadZurLogDatei> [-V <LogVerbosität>]] ./<osmp_executable> [<param1> <param2> ...]
 * entsprechen, wird printUsage() aufgerufen und das Programm mit EXIT_FAILURE beendet.
 * Achtung: exec_args_index kann == argc sein, nämlich dann, wenn keine Argumente für die OSMP-Executable übergeben werden.
 * Dies muss von der aufrufenden Funktion abgefangen werden.#define SHARED_MEMORY_NAME "/shared_memory"
 *
 * @param[in] argc              Die Anzahl der gesamten Kommandozeilenargumente, die an dieses Programm übergeben wurden.
 * @param[in] argv              Zeiger auf die gesamten Kommandozeilenargumente, die an dieses Programm übergeben wurden.
 * @param[out] processes        Zeiger auf die Anzahl der Prozesse, die gestartet werden sollen.
 * @param[out] log_file         Zeiger auf den Namen des Logfiles. Wird auf NULL gesetzt, wenn argv keine Logdatei angibt.
 * @param[out] verbosity        Zeiger auf die Log-Verbosität. Wird auf 1 gesetzt, wenn argv keinen oder einen ungültigen Wert enthält.
 * @param[out] executable       Zeiger auf den Namen der Executable. Wird auf NULL gesetzt, wenn in den Argumenten nicht gesetzt oder leer.
 * @param[out] exec_args_index  Zeiger auf den Index in Bezug auf argv, an dem das erste an die OSMP-Executable zu übergebende Argument steht (den Namen der Executable nicht eingeschlossen).
 */
void parse_args(int argc, char* argv[], int* processes, char** log_file, int* verbosity, char** executable, int* exec_args_index) {

    // Überprüfen, ob ausreichend Argumente vorhanden sind
    if (argc < 3) {
        printUsage();
        exit(EXIT_FAILURE);
    }

    // Interpretation der ersten Pflichtangabe
    *processes = atoi(argv[1]);

    printf("# of processes: %d\n", *processes);

    int i = 2;
    // Durchlaufen der Argumente
    while (i < argc) {
        if (strcmp(argv[i], "-L") == 0) {
            if (i + 1 >= argc) {
                printUsage();
                exit(EXIT_FAILURE);
            }
            // Interpretation der optionalen Log-Datei
            *log_file = argv[i + 1];
            if(is_whitespace(*log_file)) {
                *log_file = NULL;
            }
            // maximal erlaubte Länge des Pfads zur Logdatei überprüfen (Nullbyte einrechnen)
            if((strlen(*log_file)+1) > MAX_PATH_LENGTH) {
                print_logfile_condition();
                exit(EXIT_FAILURE);
            }
            printf("Logfile: %s\n", *log_file);
            i += 2;
        } else if (strcmp(argv[i], "-V") == 0) {
            if (i + 1 >= argc) {
                printUsage();
                exit(EXIT_FAILURE);
            }
            // Interpretation der optionalen Log-Verbosität
            *verbosity = atoi(argv[i + 1]);
            printf("Verbosity: %d\n", *verbosity);
            i += 2;
        } else {
            // Wenn kein optionales Argument erkannt wurde, brich die Schleife ab
            break;
        }
    }

    // Überprüfen, ob das Argument für die auszuführende Datei vorhanden ist
    if (i >= argc) {
        printUsage();
        exit(EXIT_FAILURE);
    }

    // Interpretation des Pfades zur OSMP-Executable
    *executable = argv[i];
    i++;

    // Die restlichen Parameter sind Argumente für die OSMP-Executable
    *exec_args_index = i;
}

void set_shm_name(void)  {
    int pid = getpid();
    // Länge von pid
    long length_prefix = strlen("/shared_memory_");
    int length_pid = snprintf(NULL, 0, "%d", pid);
    // Präfix + PID + Nullbyte
    unsigned long total_length = (unsigned long)(length_prefix + length_pid + 1);
    // Allokiere ausreichend Speicherplatz für zusammengesetzten Namen
    shared_memory_name = calloc(total_length, sizeof(char));
    // Konkateniere Strings
    snprintf(shared_memory_name, total_length, "/shared_memory_%d", pid);
}

/** Initialisiert barrier mit der angegebenen Größe count und allen Standardwerten.
 *
 * @param barrier Zeiger auf die Barrier, die initialisiert werden soll.
 * @param count   Anzahl der Prozesse, die an der Barriere warten können/müssen.
 * @return        OSMP_SUCCESS im Erfolgsfall, sonst OSMP_FAILURE.
 */
int barrier_init(barrier_t *barrier, int count) {
    int return_value;

    return_value = init_shared_mutex(&(barrier->mutex));
    if(return_value != OSMP_SUCCESS) {
        log_to_file(3, "Couldn't initialize barrier mutex.");
        return OSMP_FAILURE;
    }

    return_value = init_shared_cond_var(&(barrier->convar));
    if(return_value != OSMP_SUCCESS) {
        log_to_file(3, "Couldn't initialize barrier condition var.");
        return OSMP_FAILURE;
    }

    barrier->counter = count;
    barrier->cycle = 0;
    barrier->valid = 1;
    return OSMP_SUCCESS;
}

/**
 * Hilfsmethode, um formatierte Strings als Fehlermeldungen bei der Initialisierung von postbox_utiliites zu loggen.
 * @param format_str   Formatierungsstring für die Fehlermeldung (muss genau ein %d als Formatierungsanweisung enthalten).
 * @param process_rank Rang des Prozesses, bei dessen Initialisierung ein Fehler auftritt (wird für %d eingesetzt).
 */
void log_pb_util_init_error(const char* format_str, int process_rank) {
    // Ermittle nötige String-Länge
    int len = snprintf(NULL, 0, format_str, process_rank);
    // Setze Fehlernachricht zusammen
    char buf[len];
    snprintf(buf, (unsigned long)len, format_str, process_rank);
    // Logge Nachricht
    log_to_file(3, buf);
}

/**
 * Setzt alle initialen Werte im fixen Teil des Shared Memory, außer folgende:
 * - Der Logging-Mutex wird durch den Logger gesetzt.
 * - Die Prozess-Infos werden in start_all_executables() gesetzt.
 * @param shm_ptr Pointer auf den Shared Memory.
 * @param processes Anzahl der Prozesse.
 * @param verbosity Logging-Verbosität.
 */
void init_shm(shared_memory* shm_ptr, int processes, int verbosity) {
    int return_value;

    shm_ptr->size = processes;

    // Logging-Mutex wird im Logger gesetzt

    // Notiere freie Slots in Liste
    for(int i=0; i<OSMP_MAX_SLOTS; i++) {
        shm_ptr->free_slots[i] = i;
    }

    // Initialisiere shared Semaphore für freie Slots
    return_value = sem_init(&(shm_ptr->sem_shm_free_slots), 1, OSMP_MAX_SLOTS);
    if(return_value != 0) {
        log_to_file(3, "Error on initializing Semaphore sem_shm_free_slots.");
        exit(EXIT_FAILURE);
    }

    // Initialisiere Mutex für die Liste mit freien Slots
    return_value = init_shared_mutex(&(shm_ptr->mutex_shm_free_slots));
    if(return_value != OSMP_SUCCESS) {
        log_to_file(3, "Error on initializing Mutex mutex_shm_free_slots.");
        exit(EXIT_FAILURE);
    }

    // Initialisiere Slots
    for(int i=0; i<OSMP_MAX_SLOTS; i++) {
        memset(&(shm_ptr->slots[i]), '\0', sizeof(message_slot));
    }

    // Initialisiere Mutex für gather
    return_value = init_shared_mutex(&(shm_ptr->gather_t.mutex));
    if(return_value != OSMP_SUCCESS) {
        log_to_file(3, "Couldn't initialize Mutex for gather");
        exit(EXIT_FAILURE);
    }

    // Initialisiere Condition-Variable für Gather
    return_value = init_shared_cond_var(&(shm_ptr->gather_t.condition_variable));
    if(return_value != OSMP_SUCCESS) {
        log_to_file(3, "Couldn't initialize condition var for gather");
        exit(EXIT_FAILURE);
    }

    // Initialisiere Counter für gather
    shm_ptr->gather_t.counter = 0;

    // Initialisiere Barrier
    return_value = barrier_init(&(shm_ptr->barrier), processes);
    if(return_value != OSMP_SUCCESS) {
        log_to_file(3, "Couldn't initialize barrier");
        exit(EXIT_FAILURE);
    }

    // Setze Logging-Infos
    strncpy(shm_ptr->logfile, get_logfile_name(), MAX_PATH_LENGTH);
    shm_ptr->verbosity = (unsigned int)verbosity;

    // Setze Process Infos
    process_info* info = &(shm_ptr->first_process_info);
    for(int i=0; i<processes; i++) {
        info->rank = i;
        // PID wird in start_all_executables() gesetzt

        // Initialisiere postbox_utilities
        postbox_utilities* pb_util;

        for(int j=0; j<OSMP_MAX_MESSAGES_PROC; j++) {
            pb_util->postbox[j] = NO_MESSAGE;
        }

        pb_util->in_index = 0;

        return_value = init_shared_mutex(&(pb_util->mutex_proc_in));
        if(return_value != OSMP_SUCCESS) {
            log_pb_util_init_error("Couldn't initialize mutex_proc_in in postbox_utilities of process # %d", i);
        }

        pb_util->out_index = OSMP_MAX_MESSAGES_PROC;

        return_value = init_shared_mutex(&(pb_util->mutex_proc_out));
        if(return_value != OSMP_SUCCESS) {
            log_pb_util_init_error("Couldn't initialize mutex_proc_out in postbox_utilities of process # %d", i);
        }

        return_value = sem_init(&(pb_util->sem_proc_empty), 1, OSMP_MAX_MESSAGES_PROC);
        if(return_value != 0) {
            log_pb_util_init_error("Couldn't initialize sem_proc_empty in postbox_utilities of process # %d", i);
        }

        return_value = sem_init(&(pb_util->sem_proc_full), 1, OSMP_MAX_MESSAGES_PROC);
        if(return_value != 0) {
            log_pb_util_init_error("Couldn't initialize sem_proc_full in postbox_utilities of process # %d", i);
        }

        // Semaphore muss anfangs blockieren, bis zu lesende Nachrichten vorliegen
        for(int j=0; j<OSMP_MAX_MESSAGES_PROC; j++) {
            sem_wait(&(pb_util->sem_proc_full));
        }

        // Initialisiere Gather-Slot
        memset(&(info->gather_slot), '\0', sizeof(message_slot));

        // Setze Zeiger auf nächste Process-Info
        info++;
    }
}

int main (int argc, char **argv) {
    int processes, verbosity = 1, exec_args_index;
    char* log_file;
    char* executable;

    set_shm_name();

    parse_args(argc, argv, &processes, &log_file, &verbosity, &executable, &exec_args_index);

    // Größe des SHM berechnen
    shm_size = calculate_shared_memory_size(processes);

    int shared_memory_fd = shm_open(shared_memory_name, O_CREAT | O_RDWR, 0666);
    if (shared_memory_fd==-1){
        return -1;
    }

    int ftruncate_result = ftruncate(shared_memory_fd, shm_size);
    printf("shm_size: %d B\n", shm_size);
    if(ftruncate_result == -1){
        return -1;
    }
    shared_memory *shm_ptr = mmap(NULL, (size_t) shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    if (shm_ptr == MAP_FAILED){
        return -1;
    }
    logging_init_parent(shm_ptr, log_file, verbosity);

    init_shm(shm_ptr, processes, verbosity);

    OSMP_Init_Runner(shared_memory_fd, shm_ptr, shm_size);

    // Erstes Argument muss gemäß Konvention (execv-Manpage) Name der auszuführenden Datei sein.
    char ** arguments = argv + exec_args_index -1;
    int starting_result = start_all_executables(processes, executable, arguments, shm_ptr);
    if(starting_result!=0){
        return -1;
    }
    for (int j = 0; j < processes; ++j) {
        int status;
        pid_t pid_child =  wait(&status);
        if (pid_child==-1){
            log_to_file(3, "Problem by waiting");
        }
        if ( WIFEXITED(status)&&WEXITSTATUS(status)!=0 ) {
            printf("Child returned failure code.\n");
        }
    }

    int free_result = freeAll(shared_memory_fd, shm_ptr);

    if(free_result == -1){
        return -1;
    }
    return 0;
}

