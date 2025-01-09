#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {

    const char *commands[][4] = {
        {"/bin/ls","-l",NULL},
        {"/bin/echo","hello","world",NULL},
        {"/usr/bin/whoami",NULL},
        {NULL}
    };

    int i=0;
    int arr_len = sizeof(commands) - 1;
    printf("arr_len==%d", arr_len);
    while (commands[i][0] != NULL) {
        printf("i==%d\n", i);

        pid_t pid = fork();

        if (pid==0) {
            printf("child execv cmd");
            execv(commands[i][0], (char*const*)commands[i]);
            // if success: wont reach here...
            perror("execv failed");
            exit(1);
        } else if (pid > 0) {
            int status;
            wait(&status);
            printf("command %s completed with status==%d\n", commands[i][0], WEXITSTATUS(status));
        } else {
            perror("fork failed");
            exit(1);
        }

        //
        i++;
    }
   
    return 0;
}