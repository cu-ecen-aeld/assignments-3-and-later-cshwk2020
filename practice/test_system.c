#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("arg0==%s", argv[1]);
    int rc = system(argv[1]);
    printf("rc==%d\n", rc);
    return rc;
}