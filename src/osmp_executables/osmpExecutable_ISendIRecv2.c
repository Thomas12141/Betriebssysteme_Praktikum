/******************************************************************************
* FILE: osmpExecutable_SendRecv2.c
* DESCRIPTION:
* OSMP program with many OSMP_ISend/OSMP_Recv calls
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

    OSMP_Request myrequest[size-1];
    for(int i=0; i<size-1; i++) {
        myrequest[i] = NULL;
    }

    int bufin[1], bufout[size-1], len;
    bufin[0] = rank;

    int counter = 0;
    for (int i = 0; i < size; ++i) {
        if(i==rank){

        } else{
            rv = OSMP_CreateRequest( &(myrequest[counter] ));
            rv = OSMP_ISend(bufin, 1, OSMP_INT, i, myrequest[counter]);
            counter++;
            if(rv == OSMP_FAILURE){
                OSMP_Finalize();
                printf("OSMP_ISend returned error for rank %d with destination %d\n", rank, i);
                return -1;
            }
        }
    }

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



    for(int i=0; i<size-1; i++) {
        rv = OSMP_Wait( myrequest[i] );
        rv = OSMP_RemoveRequest(myrequest[i]);
    }

    rv = OSMP_Finalize();

    if(rv == OSMP_FAILURE){
        printf("OSMP_Finalize returned error for rank %d\n", rank);
        return -1;
    }

    return 0;
}
