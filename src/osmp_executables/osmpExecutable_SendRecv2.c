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
    rv = OSMP_Init(&argc, &argv);
    if(rv != OSMP_SUCCESS){
        printf("OSMP_Init: returned error number %d\n", rv);
        return -1;
    }
    rv = OSMP_Size(&size);
    if(rv != OSMP_SUCCESS){
        printf("OSMP_Size: returned error number %d\n", rv);
        return -1;
    }
    rv = OSMP_Rank(&rank);
    if(rv != OSMP_SUCCESS){
        printf("OSMP_Rank: returned error number %d\n", rv);
        return -1;
    }
    int bufin[1], bufout[size-1], len;
    bufin[0] = rank;
    for (int i = 0; i < size; ++i) {
        if(i==rank){
            for(int j = 0; j<size-1; j++){
                rv = OSMP_Recv(&(bufout[j]), 1, OSMP_INT, &source, &len);
                if(rv == OSMP_FAILURE){
                    OSMP_Finalize();
                    printf("OSMP_Recv returned error for rank %d\n", rank);
                    return -1;
                }
            }

            printf("OSMP process %d received the following [", rank);
            for (int j = 0; j < size-2; ++j) {
                printf("%d:",bufout[j]);
            }
            printf("%d]\n",bufout[size-2]);
        } else{
            rv = OSMP_Send(bufin, 1, OSMP_INT, i);

            if(rv == OSMP_FAILURE){
                OSMP_Finalize();
                printf("OSMP_Send returned error for rank %d with destination %d\n", rank, i);
                return -1;
            }
        }
    }
    rv = OSMP_Finalize();
    if(rv == OSMP_FAILURE){
        printf("OSMP_Finalize returned error for rank %d\n", rank);
        return -1;
    }
    return 0;
}
