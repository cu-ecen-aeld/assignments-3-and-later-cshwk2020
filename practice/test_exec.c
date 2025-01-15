#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    //printf("args==%s", argv[1]);
 
    char *args[] = {"/bin/ls", "-l", NULL};
    printf("before exec");
    printf("pid==%d\n", getpid());
        
    execvp(args[0], args);

    perror("exec failed");
    return 1;
}