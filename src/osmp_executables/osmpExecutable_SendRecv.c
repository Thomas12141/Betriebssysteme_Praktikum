/******************************************************************************
* FILE: osmpExecutable_SendRecv.c
* DESCRIPTION:
* OSMP program with a simple pair of blocking OSMP_Send/OSMP_Recv calls
*
* LAST MODIFICATION: Darius Malysiak, March 21, 2023
******************************************************************************/
#include <stdio.h>

#include "../osmp_library/OSMP.h"

int main(int argc, char *argv[]) {
    int rv, size, rank, source;
    int bufin[2], bufout[2], len;
    rv = OSMP_Init(&argc, &argv);
    if(rv != 0){
        printf("OSMP_Init: returned error namuber %d\n", rv);
    }
    rv = OSMP_Size(&size);
    if(rv != 0){
        printf("OSMP_Size: returned error number %d\n", rv);
    }
    rv = OSMP_Rank(&rank);
    if(rv != 0){
        printf("OSMP_Rank: returned error number %d\n", rv);
    }
    if (size != 2) {
        printf("You have to start runner with 2 processes exactly.\n");
        return -1;
    }
    if (rank == 0) {
        // OSMP process 0
        bufin[0] = 4711;
        bufin[1] = 4712;
        rv = OSMP_Send(bufin, 2, OSMP_INT, 1);
        printf("OSMP_Send return %d\n",rv);
    } else {
        // OSMP process 1
        rv = OSMP_Recv(bufout, 2, OSMP_INT, &source, &len);
        printf("OSMP_Recv return %d\n",rv);
        printf("OSMP process %d received %d byte from %d [%d:%d] \n", rank, len, source, bufout[0], bufout[1]);
    }
    rv = OSMP_Finalize();
    printf("%d\n", rv);
    return 0;
}
