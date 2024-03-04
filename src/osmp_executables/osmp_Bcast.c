/******************************************************************************
* FILE: osmp_Bcast.c
* DESCRIPTION:
* OSMP program with simple OSMP_Bcast call
*
* LAST MODIFICATION: Darius Malysiak, March 21, 2023
******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "../osmp_library/OSMP.h"

int main(int argc, char *argv[]) {
    int rv, size, rank;
    char buffer[OSMP_MAX_PAYLOAD_LENGTH], count;
    rv = OSMP_Init(&argc, &argv);
    rv = OSMP_Size(&size);
    rv = OSMP_Rank(&rank);
    if (size < 2) {/* Fehlerbehandlung */
    }
    if (rank == 1) { // OSMP process 1, this is the broadcasting ”root”
        count = 12; // number of bytes
        memcpy(buffer, "Hello World" , (size_t) count); // copy data to buffer and broadcast …
        rv = OSMP_Bcast(buffer, count, OSMP_BYTE, true, NULL, NULL);
    } else { // all other OSMP processes receive the message
        count = 12; // number of bytes
        int source, len;
        rv = OSMP_Bcast(buffer, count, OSMP_BYTE, false, &source, &len);
        // use data in buffer
    }
    rv = OSMP_Finalize();
    printf("%d\n", rv);
    return 0;
}
