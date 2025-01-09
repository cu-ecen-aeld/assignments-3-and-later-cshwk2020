#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    printf("before fork: process pid==%d\n", getpid());
    
    pid_t pid = fork();

    if (pid==0) {
        printf("child process pid==%d\n", getpid());
        //_exit(1);
    } else if (pid > 0) {
        printf("parent process pid==%d\n", getpid());
         
    } else {
        printf("EEROR: fork failed...");
    }

}