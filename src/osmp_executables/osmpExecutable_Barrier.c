/******************************************************************************
* FILE: osmpExecutable_Barrier.c
* DESCRIPTION:
* OSMP program using OSMP_Barrier(). Every process waits a random time (1-10 s)
* and then calls Barrier.
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "../osmp_library/OSMP.h"

int main(int argc, char *argv[]) {
    int rv, size=0, rank=0, wait_time;
    rv = OSMP_Init(&argc, &argv);
    rv = OSMP_Size(&size);
    rv = OSMP_Rank(&rank);
    // Seed für Zufallszahlen setzen
    srand((unsigned int)(time(NULL)+rank));
    // Setze wait_time auf eine zufällige Zahl zwischen 1 und 10
    int min = 1;
    int max = 10;
    int pid = getpid();
    wait_time = min + rand() % (max - min + 1);
    // Warte eine zufällige Zeitspanne
    printf("Prozess %d wartet %d Sekunden...\n", pid, wait_time);
    sleep((unsigned int)wait_time);
    // Rufe OSMP_Barrier(), um zu warten, bis alle anderen Prozesse ebenfalls an diesem Punkt angelangt sind.
    printf("Prozess %d ruft Barrier()...\n", pid);
    rv = OSMP_Barrier();
    printf("Prozess %d: rv von Barrier() = %d\n", pid, rv);
    rv = OSMP_Finalize();
    printf("rv = %d\n", rv);
    return 0;
}
