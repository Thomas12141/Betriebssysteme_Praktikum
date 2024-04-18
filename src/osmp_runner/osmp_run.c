#include "osmp_run.h"
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#include "../osmp_library/logger.h"

#define SHARED_MEMORY_NAME "/shared_memory"
#define SHARED_MEMORY_SIZE 1024

int start_all_executables(int number_of_executables, char* executable, char ** arguments){
    for (int i = 0; i < number_of_executables; ++i) {
        pid_t pid = fork();
        for(int j = 0; arguments[j] != NULL; j++) {
            printf("%s ", arguments[j]);
        }
        puts("");
        if (pid < 0) {
            log_to_file(3, __TIMESTAMP__, "failed to fork");
            return -1;
        } else if (pid == 0) {
            execv(executable, arguments);
            log_to_file(3, __TIMESTAMP__, "execv failed");
            return -1;
        }
    }
    return 0;
}

int freeAll(int shared_memory, char * shm_ptr){
    int result = munmap(shm_ptr, SHARED_MEMORY_SIZE);
    if(result==-1){
        log_to_file(3, __TIMESTAMP__, "Couldn't unmap memory.");
        return -1;
    }
    result = close(shared_memory);
    if(result==-1){
        log_to_file(3, __TIMESTAMP__, "Couldn't close file descriptor memory.");
        return -1;
    }
    result = shm_unlink(SHARED_MEMORY_NAME);
    if(result==-1){
        log_to_file(3, __TIMESTAMP__, "Couldn't unlink file name.");
        return -1;
    }
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
 * Dies muss von der aufrufenden Funktion abgefangen werden.
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


int main (int argc, char **argv) {
    int processes, verbosity, exec_args_index;
    char *log_file = NULL, *executable;
    parse_args(argc, argv, &processes, &log_file, &verbosity, &executable, &exec_args_index);

    logging_init(log_file, verbosity);

    int shared_memory_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);

    if (shared_memory_fd==-1){
        log_to_file(3, __TIMESTAMP__, "Failed to open shared memory.");
        return -1;
    }
    int ftruncate_result = ftruncate(shared_memory_fd, SHARED_MEMORY_SIZE);

    if(ftruncate_result == -1){
        log_to_file(3, __TIMESTAMP__, "Failed to truncate.");
        return -1;
    }
    char *shm_ptr = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);

    if (shm_ptr == MAP_FAILED){
        log_to_file(3, __TIMESTAMP__, "Failed to map memory.");
        return -1;
    }

    // Erstes Argument muss gemäß Konvention (execv-Manpage) Name der auszuführenden Datei sein.
    char ** arguments = argv + exec_args_index -1;
    int starting_result = start_all_executables(processes, executable, arguments);

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

