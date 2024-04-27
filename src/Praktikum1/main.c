#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int main(void) {
    pid_t pid = fork();
    if (pid == -1) {
        printf("Failed to fork.");

    } else if (pid != 0) {
        int status;
        printf("Waiting for child\n");
        pid_t w = waitpid(pid, &status, 0);
        if (w==-1){
            printf("Problem by wainting\n");
        }
        if ( WIFEXITED(status)&&WEXITSTATUS(status)!=0 ) {
            printf("Child returned failure code.\n");
        }
    } else {
        execl("./echoall", "echoall", "Hello", "World", NULL);
        perror("execl failed");
        return -1;
    }
    return 0;
}