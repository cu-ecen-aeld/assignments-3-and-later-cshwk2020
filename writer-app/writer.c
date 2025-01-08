#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>


void log_message(const char *format, ...) {

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    syslog(LOG_USER | LOG_DEBUG, format, args);

}

int ensure_folder_exists(const char *dir) {

    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        if (mkdir(dir, 0755) == -1) {
            perror("ERROR create dir");
            log_message("ERROR: failed to create directory %s\n", dir);
            return 0;
        }

    }
    return 1;
}

int main(int argc, char *argv[]) {

    openlog(NULL, 0, LOG_USER);


    if (argc != 3) {
        //syslog(LOG_DEBUG, "Error: Missing arguments. Usage: %s <file_path> <string>", argv[0]);
        log_message("Error: Missing arguments. Usage: %s <file_path> <string>\n", argv[0]);
        closelog();
        exit(1);
    }

    const char *file_path = argv[1];
    const char *string_to_write = argv[2];

    const char *dir_path = strdup(file_path);
    if (dir_path != NULL) {
        char *directory = dirname((char *)dir_path);
        if (!ensure_folder_exists(directory)) {
            free((void*)dir_path);
            dir_path = NULL;
        }
    } 

     

    FILE *file = fopen(file_path, "w");
    if (!file) {
        // syslog(LOG_DEBUG, "Error: Cannot open or create file '%s': %s", file_path, strerror(errno));
        log_message("Error: Cannot open or create file '%s': %s", file_path, strerror(errno));

        if (dir_path != NULL) {
            free((void*)dir_path);
            dir_path = NULL;
        }
        closelog();
        exit(1);
    }

    int rc = fprintf(file, "%s\n", string_to_write);
    //printf("rc==%d\n", rc);
    if (rc < 0) {
        //syslog(LOG_DEBUG, "Error: Failed to write to file '%s': %s", file_path, strerror(errno));
        log_message("Error: Failed to write to file '%s': %s\n", file_path, strerror(errno));
        
        if (dir_path != NULL) {
            free((void*)dir_path);
            dir_path = NULL;
        }

        fclose(file);
        closelog();
        exit(1);
    }

    if (dir_path != NULL) {
        free((void*)dir_path);
        dir_path = NULL;
    }
    fclose(file);
    closelog();
    exit(0);
}