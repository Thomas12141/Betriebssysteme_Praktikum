#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>

//
// Created by fidor on 22.05.2024.
//
int main(void) {
    int count = 30;
    errno = 0;

    for (int i = count; i >= 0; --i) {
        int pid = fork();
        if (pid == 0) {
            // Child process
            printf("Child process with i = %d\n", i);
            execl("./cmake-build-debug/osmp_run", "./cmake-build-debug/osmp_run", "50", "./cmake-build-debug/osmpExecutable_Barrier", "Test", NULL);
            // If execl fails, print error and exit
            perror("execl failed");
            exit(errno);
        } else if (pid < 0) {
            // Fork failed
            perror("fork failed");
            exit(errno);
        } else {
            // Parent process
            printf("Parent process created a child with i = %d\n", i);
        }
    }

    // Parent process waits for all child processes to finish
    for (int i = count; i >= 0; --i) {
        wait(NULL);
    }

    return 0;
}
