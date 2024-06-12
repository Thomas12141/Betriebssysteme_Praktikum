/******************************************************************************
* FILE: osmpExecutable_GatherLoop.c
* DESCRIPTION:
* OSMP program with Gather in a loop
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../osmp_library/OSMP.h"

#define LOOPS 100

int main(int argc, char *argv[]) {
    int rv, size=0, rank=0;
    rv = OSMP_Init(&argc, &argv);
    rv = OSMP_Size(&size);
    rv = OSMP_Rank(&rank);
    int elems = 2*size;
    int bufin[2], bufout[elems];
    memset(bufout, '\0', sizeof(int) * (unsigned long) (elems));
    int recv;
    if (size < 2) {
        printf("Mindestens 2 Prozesse für Gather-Kommunikation benötigt! Vorhandene Prozesse: %d\n", size);
        exit(EXIT_FAILURE);
    }

    printf("pid %d\n", getpid());

    OSMP_Barrier();

    // Prozess mit Rang 0 ist empfangender Prozess
    recv = 0;
    for(int i=0; i<LOOPS; i++) {
        // Verwende Schleifendurchlauf und eigene PID als Payload
        bufin[0] = i;
        bufin[1] = getpid();
        // Sende/Empfange mit OSMP_Gather
        rv = OSMP_Gather(bufin, 2, OSMP_INT, bufout, elems, OSMP_INT, recv);
        if (rank == recv) {
            printf("OSMP process %d received %d messages via Gather:\n", rank, size);
            for(int j=0; j<size; j++) {
                printf("[%3d] %3d %3d\n", j, bufout[2*j], bufout[2*j + 1]);
            }
        }
    }

    rv = OSMP_Finalize();
    printf("rv = %d\n", rv);
    return 0;
}
