#include "osmp_run.h"

int main (int argc, char **argv) {
    printf("Hello, World!\n");

    for (char **p = argv; *p; p++) {
        printf("Argument: %s\n", *p);
    }

    return 0;
}