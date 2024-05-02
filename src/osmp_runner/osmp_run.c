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

int start_all_executables(int number_of_executables, char* executable, char ** arguments, char * shm_ptr){
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
            // Schreibe PIDs in Shared Memory (erster Int ist für Size => i+1)
            unsigned long offset = (unsigned long) (i + 1) * sizeof(int);
            memcpy(shm_ptr + offset, &pid, sizeof(int));
        }
    }
    return 0;
}

int freeAll(int shared_memory, char * shm_ptr){
    int result = munmap(shm_ptr, (size_t) shm_size);
    if(result==-1){
        log_to_file(3, __TIMESTAMP__, "Couldn't unmap memory.");
        return -1;
    }
    result = close(shared_memory);
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
 * Setzt alle initialen Werte im Shared Memory.
 * @param shm_ptr Pointer auf den Shared Memory.
 * @param processes Anzahl der Prozesse.
 * @param verbosity Logging-Verbosität.
 */
void init_shm(char* shm_ptr, int processes, int verbosity) {
    // Anzahl der Prozesse stehen am Anfang des SHM.
    memcpy(shm_ptr, &processes, sizeof(processes));

    // PIDs / Ranks werden in start_all_executables() gesetzt

    // TODO: Mutex

    // Setze freie Slots
    // Offset zur Liste der freien Slots: size, pids überspringen => 1+n ints; mutex überspr.
    int free_slots_list_offset = (1 + processes) * (int)sizeof(int) + (int)sizeof(pthread_mutex_t);
    // Offset zum ersten Nachrichtenslot:
    int first_slot_offset = free_slots_list_offset
            + get_OSMP_MAX_SLOTS() * (int)sizeof(int) // überspringe Liste mit freien Slots
            + processes * (int)sizeof(int);           // überspringe Postfächer
    for(int i=0; i<get_OSMP_MAX_SLOTS(); i++) {
        // Offset zum aktuellen Eintrag in der Liste der freien Slots
        int current_free_slot_list_offset = free_slots_list_offset + i * (int)sizeof(int);
        // Offset zum Nachrichtenslot, der eingetragen werden soll
        int current_slot_offset = first_slot_offset + i * (int)sizeof(OSMP_message);
        // Trage Nachrichtenslot in Liste ein
        memcpy(shm_ptr + current_free_slot_list_offset, &current_slot_offset, sizeof(int));
    }

    puts("Initialized free slots list");

    // Setze Postfächer auf NO_MESSAGE
    // Offset zu Postfächern: überspringe Freie-Slot-Liste
    int postbox_offset = free_slots_list_offset + get_OSMP_MAX_SLOTS() * (int)sizeof(int);
    // zu kopierende Variable
    int no_message = NO_MESSAGE;
    for(int i=0; i<processes; i++) {
        // Berechne aktuellen Postfach-Offset relativ zum ersten Postfach
        int current_postbox_offset = postbox_offset + i * (int)sizeof(int);
        memcpy(shm_ptr + current_postbox_offset, &no_message, sizeof(int));
    }

    puts("Initialized postboxes");

    // Initialisiere Nachrichtenslots: setze Flag auf SLOT_FREE und nächste Nachr. auf NO_MESSAGE
    int slot_size = (int)sizeof(OSMP_message);
    for(int i=0; i<get_OSMP_MAX_SLOTS(); i++) {
        printf("i: %d\n", i);
        // Berechne Offset des aktuellen Slots basierend auf erstem Slot
        int current_slot_offset = first_slot_offset + i * slot_size;
        printf("current_slot_offset: %d\n", current_slot_offset);
        // Pointer auf aktuellen Slot
        char* slot_pointer = shm_ptr + current_slot_offset;
        // Cast auf Message-Struct
        OSMP_message* message = (OSMP_message*)(slot_pointer);
        // Setze Werte
        message->free = SLOT_FREE;
        message->next_message = NO_MESSAGE;
        printf("msg ptr: %p\n", (void*)message);
    }

    puts("Initialized message slots");

    // Logging-Info
    strcpy(shm_ptr+shm_size-258, get_logfile_name());
    char verbosity_as_str[2];
    sprintf(verbosity_as_str, "%d", verbosity);
    strcpy(shm_ptr+shm_size-2, verbosity_as_str);
}

int main (int argc, char **argv) {
    int processes, verbosity = 1, exec_args_index;
    char *log_file = NULL, *executable;

    set_shm_name();

    parse_args(argc, argv, &processes, &log_file, &verbosity, &executable, &exec_args_index);

    // Größe des SHM berechnen
    shm_size = calculate_shared_memory_size(processes);

    logging_init_parent(log_file, verbosity);

    int shared_memory_fd = shm_open(shared_memory_name, O_CREAT | O_RDWR, 0666);
    if (shared_memory_fd==-1){
        log_to_file(3, __TIMESTAMP__, "Failed to open shared memory.");
        return -1;
    }

    int ftruncate_result = ftruncate(shared_memory_fd, shm_size);
    printf("shm_size: %d\n", shm_size);
    if(ftruncate_result == -1){
        log_to_file(3, __TIMESTAMP__, "Failed to truncate.");
        return -1;
    }
    char *shm_ptr = mmap(NULL, (size_t) shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    if (shm_ptr == MAP_FAILED){
        log_to_file(3, __TIMESTAMP__, "Failed to map memory.");
        return -1;
    }

    OSMP_Init_Runner(shared_memory_fd, shm_ptr, shm_size);

    init_shm(shm_ptr, processes, verbosity);

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

