#include <stddef.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "logger.h"
#include "OSMP.h"


pthread_mutex_t * mutex;
FILE *logging_file;
char * file_name = NULL;
int verbosity = 1;

/**
 * Öffnet die angegebene Logdatei und leert sie. Falls sie nicht existiert, wird sie neu erstellt.
 *
 * @param filename Zeiger auf den Dateipfad der Logdatei.
 */
void init_file(const char *filename) {
    // O_CREAT: Erstelle die Datei, falls sie nicht existiert
    // O_TRUNC: Lösche den Inhalt der Datei
    int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == -1) {
        perror("Fehler beim Erstellen der Datei");
        exit(EXIT_FAILURE);
    }
    close(file);
}

/**
 * Initialisiert die Log-Bibliothek. Erzeugt Logdatei, falls sie noch nicht existiert. Diese Methode ist für das Elter, das Kind hat eine eigene Methode.
 * @param shm           Pointer auf den Shared Memory.
 * @param name          Pfad zur Logdatei.
 * @param log_verbosity Logging-Verbosität (Level 1-3). Bei ungültigem Wert wird das Standard-Level 1 verwendet.
 */
void logging_init_parent(shared_memory* shm, char* name, int log_verbosity){
    pthread_mutexattr_t att;
    int result = pthread_mutexattr_init(&att);
    if (result != 0){
        perror("pthread_mutex_attr_init() error\n");
        exit(OSMP_FAILURE);
    }
    result = pthread_mutexattr_setpshared(&att, PTHREAD_PROCESS_SHARED);
    if (result != 0){
        pthread_mutexattr_destroy(&att);
        perror("pthread_mutexattr_setpshared() error\n");
        exit(OSMP_FAILURE);
    }
    result = pthread_mutex_init(&(shm->logging_mutex), &att);
    if(result < 0){
        pthread_mutexattr_destroy(&att);
        perror("pthread_mutex_init() error\n");
        exit(OSMP_FAILURE);
    }
    result = pthread_mutexattr_destroy(&att);
    if (result != 0){
        pthread_mutexattr_destroy(&att);
        perror("pthread_mutexattr_destroy() error\n");
        exit(OSMP_FAILURE);
    }

    mutex = (pthread_mutex_t *) &(shm->logging_mutex);
    // Erlaubte Werte für Verbosität: 1 bis 3
    // Bei unerlaubtem Wert wird Standard (1) verwendet
    if(log_verbosity > 1 && log_verbosity <= 3) {
        verbosity = log_verbosity;
    }

    file_name = name;
    if(name == NULL||*name == '\0'){
        // Standardwert nutzen
        file_name = "log.log";
    }

    init_file(file_name);
    logging_file = fopen(file_name, "a+");

    if(logging_file == NULL ){
        perror("Couldn't open logging file.\n");
        exit(EXIT_FAILURE);
    }
    time_t raw_time;
    struct tm * time_info;

    time ( &raw_time );
    time_info = localtime ( &raw_time );

    fprintf(logging_file,"Logfile initialized at %s.\n", asctime (time_info) );
    fputs("Log entries follow the scheme <level> - <pid> - <timestamp> - <message>.\n", logging_file);
    fflush(logging_file);
}

/**
 * Initialisiert die Log-Bibliothek für das Kind. Es speichert die verbosität und Dateiname.
 * @param shared_memory Pointer auf den Shared Memory
 * @param memory_size   Größe des Shared Memory in Bytes
 */
void logging_init_child(shared_memory* shm) {
    if (shm == NULL) {
        fprintf(stderr, "Error: shared_memory is NULL\n");
        exit(OSMP_FAILURE);
    }

    int process_number;
    OSMP_Size(&process_number);
    mutex = (pthread_mutex_t *) &(shm->logging_mutex);
    size_t file_name_length = strlen(shm->logfile);
    file_name = malloc(file_name_length + 1);
    if (file_name == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return;
    }

    strncpy(file_name, shm->logfile, file_name_length);
    file_name[file_name_length] = '\0';

    verbosity = (int)shm->verbosity;
}


/**
 * Schreibt in die Logdatei.
 *
 * @param level     Logging-Level des Eintrags (1-3).
 * @param message   Zu loggende Nachricht.
 */
void log_to_file(int level, char* message){
    int return_code;
    if(file_name == NULL){
        printf("You must first initialize the logger.\n");
        return;
    }

    // Logge nur das festgelegte Level und alles darunter.
    if(level > verbosity) {
        return;
    }

    return_code = pthread_mutex_lock(mutex);
    if(return_code!=0){
        printf("Failed to acquire lock, error: %d\n", return_code);
        return;
    }
    logging_file = fopen(file_name, "a+");

    time_t raw_time;
    struct tm * time_info;
    time ( &raw_time );
    time_info = localtime (&raw_time );
    fprintf(logging_file, "%d - %d - %s - %s.\n", level, getpid(), asctime (time_info), message);
    fclose(logging_file);
    return_code = pthread_mutex_unlock(mutex);
    if (return_code!=0){
        printf("Failed to release lock, error: %d\n", return_code);
    }
}

/**
 * Schließe das Logging ab. Die Logdatei wird geschlossen.
 */
void logging_close(void){
    logging_file = NULL;
    free(file_name);
}

/**
 * Gibt den Dateinamen zum Loggen zurück.
 * @return Der Name der Datei als String.
 */
char* get_logfile_name(void){
    return file_name;
}
