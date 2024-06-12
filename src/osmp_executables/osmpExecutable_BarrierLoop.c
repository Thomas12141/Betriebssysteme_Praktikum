/******************************************************************************
* FILE: osmpExecutable_Barrier.c
* DESCRIPTION:
* OSMP program using OSMP_Barrier(). Every process will call OSMP_Barrier() n
* times in a loop, where x is given as a command line argument. When there is
* no CL argument, the loop will run only once.
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "../osmp_library/OSMP.h"

int main(int argc, char *argv[]) {
    int rv, size=0, rank=0, loops;
    if(argc == 0) {
        loops = 1;
    }
    else {
        loops = atoi(argv[1]);
        if(loops <= 0){
            puts("Argument muss größer als 0 sein");
            exit(-1);
        }
    }
    rv = OSMP_Init(&argc, &argv);
    rv = OSMP_Size(&size);
    rv = OSMP_Rank(&rank);

    int pid = getpid();

    for(int i=0; i<loops; i++) {
        printf("Prozess %d ist in der %d. Iteration\n", pid, i);
        // Rufe OSMP_Barrier(), um zu warten, bis alle anderen Prozesse ebenfalls an diesem Punkt angelangt sind.
        printf("Prozess %d ruft Barrier()...\n", pid);
        rv = OSMP_Barrier();
        printf("Prozess %d: rv von Barrier() = %d (--- Ende Iter. ---)\n", pid, rv);
    }

    rv = OSMP_Finalize();
    printf("rv = %d\n", rv);
    return 0;
}
