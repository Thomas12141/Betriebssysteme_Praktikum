
#ifndef BERTRIEBSSYSTEME_LOGGER_H
#define BERTRIEBSSYSTEME_LOGGER_H

#define SHARED_MEMORY_SIZE 1024

void logging_init_parent(char * name, int log_verbosity);

void log_to_file(int level, char* timestamp, char* message);

void logging_close(void);

char* get_logfile_name(void);

void logging_init_child(char * shared_memory);

#endif //BERTRIEBSSYSTEME_LOGGER_H
