#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {

    printf("before fork: process pid==%d\n", getpid());
    fflush(stdout);

    pid_t pid = fork();

    if (pid > 0) {
        printf("parent wait for child to finish...\n");
        fflush(stdout);
        wait(NULL);
        printf("parent resumed...\n");
        fflush(stdout);
    } 
    else if (pid==0) {
        printf("child process start...\n");
        fflush(stdout);
        sleep(2);
        printf("child process ended...\n");
        fflush(stdout);
    } 
    else {
        printf("EEROR: fork failed...\n");
        fflush(stdout);
    }

    return 0;

}