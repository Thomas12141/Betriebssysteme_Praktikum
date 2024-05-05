#include "osmp_run.h"
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

#include "../osmp_library/logger.h"
#include "../osmp_library/osmplib.h"

int shm_size;

/**
 * Shared memory name to be created following the scheme:
 * shared_memory_<runner_pid>
 */
char* shared_memory_name;

int start_all_executables(int number_of_executables, char* executable, char ** arguments, shared_memory* shm_ptr){
    for (int i = 0; i < number_of_executables; ++i) {
        int pid = fork();
        for(int j = 0; arguments[j] != NULL; j++) {
            printf("%s ", arguments[j]);
        }
        puts("");
        if (pid < 0) {
            log_to_file(3, __TIMESTAMP__, "failed to fork");
            return -1;
        } else if (pid == 0) {//Child process.
            execv(executable, arguments);
            log_to_file(3, __TIMESTAMP__, "execv failed");
            return -1;
        } else{
            // Initialisiere Prozess-Infos im Shared Memory
            // Offset berechnen (alle außer der 0. Prozess-Info gehen über SHM-Struct hinaus)
            process_info* info = &(shm_ptr->first_process_info) + i;
            info->rank = i;
            info->pid = pid;
            info->postbox = NO_MESSAGE;
        }
    }
    return 0;
}

int freeAll(int shm_fd, shared_memory* shm_ptr){
    int result = munmap(shm_ptr, (size_t) shm_size);
    if(result==-1){
        log_to_file(3, __TIMESTAMP__, "Couldn't unmap memory.");
        return -1;
    }
    result = close(shm_fd);
    if(result==-1){
        log_to_file(3, __TIMESTAMP__, "Couldn't close file descriptor memory.");
        return -1;
    }
    result = shm_unlink(shared_memory_name);
    if(result==-1){
        log_to_file(3, __TIMESTAMP__, "Couldn't unlink file name.");
        return -1;
    }
    log_to_file(2, __TIMESTAMP__, "Freeing shared_memory_name");
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
            continue;
        }
        return 1;
    }
    return 0;
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
                log_file = NULL;
            }
            // maximal erlaubte Länge des Pfads zur Logdatei überprüfen (Nullbyte einrechnen)
            if((strlen(*log_file)+1) > MAX_PATH_LENGTH) {
                print_logfile_condition();
                exit(EXIT_FAILURE);
            }
            i += 2;
        } else if (strcmp(argv[i], "-V") == 0) {
            if (i + 1 >= argc) {
                printUsage();
                exit(EXIT_FAILURE);
            }
            // Interpretation der optionalen Log-Verbosität
            *verbosity = atoi(argv[i + 1]);
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

/**
 * Setzt alle initialen Werte im fixen Teil des Shared Memory, außer folgende:
 * - Der Logging-Mutex wird durch den Logger gesetzt.
 * - Die Prozess-Infos werden in start_all_executables() gesetzt.
 * @param shm_ptr Pointer auf den Shared Memory.
 * @param processes Anzahl der Prozesse.
 * @param verbosity Logging-Verbosität.
 */
void init_shm(shared_memory* shm_ptr, int processes, int verbosity) {
    shared_memory* shm_struct = (shared_memory*) shm_ptr;

    shm_struct->size = processes;

    // Mutex wird im Logger gesetzt

    // Notiere freie Slots in Liste
    for(int i=0; i<OSMP_MAX_SLOTS; i++) {
        shm_struct->free_slots[i] = i;
    }

    // Initialisiere Slots
    for(int i=0; i<OSMP_MAX_SLOTS; i++) {
        shm_struct->slots[i].slot_number = i;
        shm_struct->slots[i].free = SLOT_FREE;
    }

    // Initialisiere Gather-Slot
    shm_struct->gather_slot.slot_number = processes;
    shm_struct->gather_slot.free = SLOT_FREE;
    memset(shm_struct->gather_slot.payload, '\0', OSMP_MAX_PAYLOAD_LENGTH);
    shm_struct->gather_slot.next_message = NO_MESSAGE;

    // Setze Logging-Infos
    strncpy(shm_struct->logfile, get_logfile_name(), MAX_PATH_LENGTH);
    shm_struct->verbosity = (unsigned int)verbosity;

    // Prozess-Infos werden in start_all_executables() gesetzt
}

void initialize_locks(char * shared_memory){
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    pthread_mutexattr_t att;
    pthread_mutexattr_init(&att);
    pthread_mutexattr_setpshared(&att, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&mutex, &att);
    pthread_condattr_t condition_attribute;
    pthread_condattr_init(&condition_attribute);
    pthread_condattr_setpshared(&condition_attribute, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&condition, &condition_attribute);
    pthread_condattr_destroy(&condition_attribute);
    memcpy(shared_memory, &mutex, sizeof(pthread_mutex_t));
    memcpy(shared_memory + sizeof(pthread_mutex_t), &condition, sizeof(pthread_cond_t));
}

int main (int argc, char **argv) {
    char * locks_shared_memory_string = "locks_shared_memory";
    int locks_shared_memory_fd = shm_open(locks_shared_memory_string, O_CREAT | O_RDWR, 0666);
    if (locks_shared_memory_fd == -1){
        return -1;
    }

    int locks_ftruncate_result = ftruncate(locks_shared_memory_fd, sizeof(pthread_mutex_t) + sizeof(pthread_cond_t));

    if(locks_ftruncate_result == -1){
        return -1;
    }

    char * lcoks_shm_ptr = mmap(NULL, sizeof(pthread_mutex_t) + sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED, locks_shared_memory_fd, 0);

    if (lcoks_shm_ptr == MAP_FAILED){
        return -1;
    }

    initialize_locks(lcoks_shm_ptr);

    int processes, verbosity = 1, exec_args_index;
    char *log_file = NULL, *executable;

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
            log_to_file(3, __TIMESTAMP__, "Problem by waiting");
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

