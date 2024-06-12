/******************************************************************************
* FILE: osmpExecutable_ISendIRecv.c
* DESCRIPTION:
* OSMP program with a simple pair of OSMP_ISend/OSMP_IRecv calls
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
    if(rv != OSMP_SUCCESS){
        printf("OSMP_Init: returned error number %d\n", rv);
        return -1;
    }
    rv = OSMP_Size( &size );
    if(rv != OSMP_SUCCESS){
        printf("OSMP_Size: returned error number %d\n", rv);
        return -1;
    }
    rv = OSMP_Rank( &rank );
    if(rv != OSMP_SUCCESS){
        printf("OSMP_Rank: returned error number %d\n", rv);
        return -1;
    }
    if( size != 2 ){
        printf("You have to start runner with 2 processes exactly.\n");
        return -1;
    }
    if( rank == 0 ) {
        int flag;
        // OSMP process 0
        bufin = malloc((size_t) SIZE); // check for != NULL
        len = 12; // length
        memcpy(bufin, "Hello World", (size_t) len);

        rv = OSMP_CreateRequest( &myrequest );

        rv = OSMP_ISend( bufin, len, OSMP_BYTE, 1, myrequest );

        rv = OSMP_Test(myrequest, &flag);
        printf("OSMP_Test flag %d\n", flag);
        rv = OSMP_Wait( myrequest );

        rv = OSMP_Test(myrequest, &flag);
        printf("OSMP_Test flag after OSMP_Wait %d\n", flag);

        printf("OSMP_ISend return %d\n",rv);

        rv = OSMP_RemoveRequest( &myrequest );
    }
    else {
        // OSMP process 1
        bufout = malloc((size_t) SIZE); // check for != NULL
        rv = OSMP_CreateRequest( &myrequest );
        rv = OSMP_IRecv( bufout, SIZE, OSMP_BYTE, &source, &len, myrequest );
        // do something important…
        // check if operation is completed and wait if not
        rv = OSMP_Wait( myrequest );
        // OSMP_IRecv() completed, use bufout

        printf("Received via IRecv: %s\n", bufout);

        rv = OSMP_RemoveRequest( &myrequest );
    }
    rv = OSMP_Finalize();
    printf("OSMP_Finalize return %d\n", rv);
    return 0;
}
