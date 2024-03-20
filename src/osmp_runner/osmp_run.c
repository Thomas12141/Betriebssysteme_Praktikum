#include "osmp_run.h"

int main (int argc, char **argv) {
    printf("Hello, World!\n");

    printf("Number of arguments: %d\n", argc);

    for (char **p = argv; *p; p++) {
        printf("Argument: %s\n", *p);
    }

    printf("OSMP_SUCCESS: %d\n", get_OSMP_SUCCESS());
    printf("OSMP_FAILURE: %d\n", get_OSMP_FAILURE());

    return 0;
}