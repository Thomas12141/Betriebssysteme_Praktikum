#ifndef OSMP_RUN_H
#define OSMP_RUN_H
#include <stdio.h>
#include "../osmp_library/osmplib.h"

typedef struct{
    int number_of_executables;
    int flag;
    int shared_memory_fd;
    shared_memory* shm_ptr;
} monitor_args;

#include "../osmp_library/OSMP.h"

#endif // OSMP_RUN_H
