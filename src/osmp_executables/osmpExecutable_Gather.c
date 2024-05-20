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
    long bufin[1], bufout[size];
    int recv;
    if (size < 2) {
        printf("Mindestens 2 Prozesse für Gather-Kommunikation benötigt! Vorhandene Prozesse: %d\n", size);
        exit(EXIT_FAILURE);
    }
    recv = 1;
    // Verwende eigene PID als Payload
    bufin[0] = (long)getpid();
    // Sende/Empfange mit OSMP_Gather
    rv = OSMP_Gather(bufin, 1, OSMP_LONG, bufout, size, OSMP_LONG, recv);
    if (recv== rank) {
        printf("OSMP process %d received %d messages via Gather:\n", rank, size);
        for(int i=0; i<size; i++) {
            printf("[%3d] %ld\n", i, bufout[i]);
        }
    }
    rv = OSMP_Finalize();
    printf("rv = %d\n", rv);
    return 0;
}
