/******************************************************************************
* FILE: osmpExecutable_Gather.c
* DESCRIPTION:
* OSMP program with a simple gather call.
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../osmp_library/OSMP.h"

int main(int argc, char *argv[]) {
    int rv, size=0, rank=0;
    rv = OSMP_Init(&argc, &argv);
    rv = OSMP_Size(&size);
    rv = OSMP_Rank(&rank);
    long bufin[1], bufout[size-1];
    int recv;
    if (size < 2) {
        printf("Mindestens 2 Prozesse für Gather-Kommunikation benötigt! Vorhandene Prozesse: %d\n", size);
        exit(EXIT_FAILURE);
    }
    if (rank > 0) {
        // OSMP-Prozesse > 0 (sendende Prozesse)
        recv = 1;
        bufin[rank-1] = (long)getpid();
    } else {
        // OSMP-Prozess 0 (empfangender Prozess)
        recv = 0;
    }
    // Sende/Empfange mit OSMP_Gather
    rv = OSMP_Gather(bufin, 1, OSMP_LONG, bufout, size-1, OSMP_LONG, recv);
    if (rank == 0) {
        printf("OSMP process %d received %d messages via Gather:\n", rank, size-1);
        for(int i=0; i<size-1; i++) {
            printf("[%3d] %ld\n", i, bufout[i]);
        }
    }
    rv = OSMP_Finalize();
    printf("rv = %d\n", rv);
    return 0;
}
