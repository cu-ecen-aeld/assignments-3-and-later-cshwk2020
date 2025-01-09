#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#define REDIRECT_FILE "testfile.txt"

bool do_system(const char *command);

bool do_exec(int count, ...);

bool do_exec_redirect(const char *outputfile, int count, ...);
