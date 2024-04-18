#include <stddef.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
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
 * Initialisiert die Log-Bibliothek. Erzeugt Logdatei, falls sie noch nicht existiert.
 * @param name          Pfad zur Logdatei.
 * @param log_verbosity Logging-Verbosität (Level 1-3). Bei ungültigem Wert wird das Standard-Level 1 verwendet.
 */
void logging_init(char * name, int log_verbosity){
    // Erlaubte Werte für Verbosität: 1 bis 3
    // Bei unerlaubtem Wert wird Standard (1) verwendet
    if(log_verbosity >= 1 && log_verbosity <= 3) {
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
        printf("Couldn't open logging file.\n");
        exit(EXIT_FAILURE);
    }

    fprintf(logging_file,"Logfile initialized at %s.\n", __TIMESTAMP__);
    fputs("Log entries follow the scheme <level> - <pid> - <timestamp> - <message>.\n", logging_file);
}

/**
 * Schreibt in die Logdatei.
 *
 * @param level     Logging-Level des Eintrags (1-3).
 * @param message   Zu loggende Nachricht.
 */
void log_to_file(int level, char* timestamp, char* message){
    int return_code;
    if(file_name == NULL){
        printf("You must first initialize the logger.\n");
        return;
    }

    // Logge nur das festgelegte Level und alles darunter.
    if(level > verbosity) {
        return;
    }

    return_code = pthread_mutex_lock(&mutex);
    if(return_code!=0){
        printf("Failed to acquire lock.\n");
        return;
    }
    fprintf(logging_file,"%d - %d - %s - %s.\n", level, getpid(), timestamp, message);
    return_code = pthread_mutex_unlock(&mutex);
    while (return_code!=0){
        printf("Failed to release lock.\n");
        return_code = pthread_mutex_unlock(&mutex);
    }
}

/**
 * Schließe das Logging ab. Die Logdatei wird geschlossen.
 */
void logging_close(void){
    fclose(logging_file);
    logging_file = NULL;
    pthread_mutex_destroy(&mutex);
}

/**
 * Schreibt eine Level-2-Logging-Nachricht (Speicherverwaltung) in die Logdatei.
 *
 * @param function_name Der Name der aufrufenden Funktion
 * @param message       Zusätzliche Nachricht.
 */
void log_memory_function(char* function_name, char* message, char* timestamp) {
    unsigned long string_len = strlen(function_name) + strlen(message);
    // Ausreichend großen Buffer erstellen
    char string[30 + string_len];
    sprintf(string, "%s() used memory function: %s", function_name, message);
    log_to_file(2, timestamp, string);
}


