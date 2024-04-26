/******************************************************************************
* FILE: osmpExecutable_Barrier.c
* DESCRIPTION:
* OSMP program using OSMP_Barrier()
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "../osmp_library/OSMP.h"

int main(int argc, char *argv[]) {
    puts("Test");
    int rv, size=0, rank=0, wait_time;
    rv = OSMP_Init(&argc, &argv);
    if(rv==)
    rv = OSMP_Size(&size);
    rv = OSMP_Rank(&rank);

    // Seed für Zufallszahlen setzen
    srand((unsigned int)(time(NULL)+rank));
    // Setze wait_time auf eine zufällige Zahl zwischen 1 und 10
    int min = 1;
    int max = 10;
    wait_time = min + rand() % (max - min + 1);
    // Warte eine zufällige Zeitspanne
    printf("Prozess %d wartet %d Sekunden...\n", getpid(), wait_time);
    sleep((unsigned int)wait_time);
    // Rufe OSMP_Barrier(), um zu warten, bis alle anderen Prozesse ebenfalls an diesem Punkt angelangt sind.
    rv = OSMP_Barrier();
    rv = OSMP_Finalize();
    printf("rv = %d\n", rv);
    return 0;
}
