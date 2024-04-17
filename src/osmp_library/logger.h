
#ifndef BERTRIEBSSYSTEME_LOGGER_H
#define BERTRIEBSSYSTEME_LOGGER_H

void logging_init(char * name, int log_verbosity);

void log_to_file(int level, char* message);

void logging_close();

#endif //BERTRIEBSSYSTEME_LOGGER_H
