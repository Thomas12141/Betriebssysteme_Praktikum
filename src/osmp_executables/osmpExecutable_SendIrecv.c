/******************************************************************************
* FILE: osmpExecutable_SendIrecv.c
* DESCRIPTION:
* OSMP program with a simple pair of OSMP_Send/OSMP_Irecv calls
*
* LAST MODIFICATION: Darius Malysiak, March 21, 2023
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../osmp_library/OSMP.h"

int main(int argc, char *argv[])
{
    int rv, size, rank, source, len, SIZE = 100;
    char *bufin, *bufout;
    OSMP_Request myrequest = NULL;
    rv = OSMP_Init( &argc, &argv );
    rv = OSMP_Size( &size );
    rv = OSMP_Rank( &rank );
    if( size != 2 ){ /* Fehlerbehandlung */
    }
    if( rank == 0 )
    { // OSMP process 0
        bufin = malloc((size_t) SIZE); // check for != NULL
        len = 12; // length
        memcpy(bufin, "Hello World", (size_t) len);
        rv = OSMP_Send( bufin, len, OSMP_BYTE, 1 );
    }
    else
    { // OSMP process 1
        bufout = malloc((size_t) SIZE); // check for != NULL
        rv = OSMP_CreateRequest( &myrequest );
        rv = OSMP_Irecv( bufout, SIZE, OSMP_BYTE, &source, &len, myrequest );
        // do something important…
        // check if operation is completed and wait if not
        rv = OSMP_Wait( myrequest );
        // OSMP_Irecv() completed, use bufout

        printf("%s\n", bufout);

        rv = OSMP_RemoveRequest( &myrequest );
    }
    rv = OSMP_Finalize();
    printf("%d\n", rv);
    return 0;
}
