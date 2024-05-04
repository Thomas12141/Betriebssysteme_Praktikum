
#ifndef BERTRIEBSSYSTEME_LOGGER_H
#define BERTRIEBSSYSTEME_LOGGER_H

#include "osmplib.h"

void logging_init_parent(shared_memory* shm, char * name, int log_verbosity);

void log_to_file(int level, char* timestamp, char* message);

void logging_close(void);

char* get_logfile_name(void);

void logging_init_child(shared_memory * shm);

#endif //BERTRIEBSSYSTEME_LOGGER_H
