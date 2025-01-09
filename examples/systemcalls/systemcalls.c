#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "systemcalls.h"

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    int rc = system(cmd);
    if (rc == 0) {
        return true;
    } 
    else {
        return false;
    }  
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool is_absolute_path(const char *path) {
    if (path==NULL || strlen(path)==0) {
        return false;
    }
    return (path[0] == '/');
}

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

        if (is_absolute_path(command[0])==false) {
            printf("not absolute path: %s\n", command[0]);
            fflush(stdout);
            return false;
        }

        pid_t pid = fork();

        if (pid==0) {
            printf("child execv cmd");
            fflush(stdout);

            

            execv(command[0], (char*const*)command);
            // if success: wont reach here...
            perror("execv failed");
            return false;
        } else if (pid > 0) {
            int status;
            wait(&status);
            printf("command %s completed with status==%d\n", command[0], WEXITSTATUS(status));
            fflush(stdout);
            if (status != 0) {
                return false;
            }
        } else {
            perror("fork failed");
            return false;
        }

    va_end(args);

    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

    int original_stdout = dup(STDOUT_FILENO);
    if (original_stdout == -1) {
        perror("dup original stdout failed");
        return false;
    }

    /* 
    if (dup2(original_stdout, STDOUT_FILENO) == -1) {
        perror("dup2 restore stdout failed");
        return false;
    }
    */

    int fd = open(REDIRECT_FILE, O_WRONLY|O_TRUNC|O_CREAT, 0644);
    if (fd < 0) { perror("open"); return false; }
    if (dup2(fd, 1) < 0) { perror("dup2"); return false; }
     
        //
        pid_t pid = fork();

        if (pid==0) {
            //printf("child execv cmd");
            if (is_absolute_path(command[0])==false) {
                printf("not absolute path: %s\n", command[0]);
                fflush(stdout);
                
                if (dup2(original_stdout, STDOUT_FILENO) == -1) {
                    perror("dup2 restore stdout failed");
                    return false;
                }
                close(original_stdout);

                close(fd);

                
                return false;
            }

            execv(command[0], (char*const*)command);
            // if success: wont reach here...
            perror("execv failed");

            if (dup2(original_stdout, STDOUT_FILENO) == -1) {
                    perror("dup2 restore stdout failed");
                    return false;
            }
            close(original_stdout);
            close(fd);

            //exit(1);
            return false;
        } else if (pid > 0) {
            int status;
            wait(&status);
            //printf("command %s completed with status==%d\n", command[0], WEXITSTATUS(status));
            if (status != 0) {
                if (dup2(original_stdout, STDOUT_FILENO) == -1) {
                    perror("dup2 restore stdout failed");
                    return false;
                }
                close(original_stdout);
                close(fd);
                return false;
            }
        } else {
            perror("fork failed");

            if (dup2(original_stdout, STDOUT_FILENO) == -1) {
                    perror("dup2 restore stdout failed");
                    return false;
            }
            close(original_stdout);
            close(fd);
            return false;
        }


    if (dup2(original_stdout, STDOUT_FILENO) == -1) {
        perror("dup2 restore stdout failed");
        return false;
    }
    close(original_stdout);
    
    close(fd);

    va_end(args);

    return true;
}


/*
do_system("echo this is a test > " REDIRECT_FILE ),
do_system("echo \"home is $HOME\" > " REDIRECT_FILE) 
do_exec(2, "echo", "Testing execv implementation with echo") 
do_exec(3, "/usr/bin/test","-f","echo") 
do_exec(3, "/usr/bin/test","-f","/bin/echo") 
do_exec_redirect(REDIRECT_FILE, 3, "/bin/sh", "-c", "echo home is $HOME");
do_exec_redirect(REDIRECT_FILE, 2, "/bin/echo", "home is $HOME");
*/

int __main(int argc, char *argv[]) {

    ////int fd = open("REDIRECT_FILE, O_WRONLY|O_TRUNC|O_CREAT, 0644);
    ////if (fd < 0) { perror("open"); abort(); }
    ////if (dup2(fd, 1) < 0) { perror("dup2"); exit(1); }
     

    // do_system("/bin/ls -l");
    // do_system("echo this is a test > " REDIRECT_FILE );
    int rc = do_exec(2, "echo", "Testing execv implementation with echo");
    //do_exec(3, "/usr/bin/test","-f","/bin/echo");
    // int rc = do_exec(3, "/usr/bin/test","-f","echo");
    //int rc = do_exec_redirect(REDIRECT_FILE, 3, "/bin/sh", "-c", "echo home is $HOME");
    //int rc=do_exec_redirect(REDIRECT_FILE, 3, "/bin/sh", "-c", "echo home is $HOME");
    //int rc = do_exec(2, "echo", "Testing execv implementation with echo");
    printf("main::rc==%d\n", rc);
    fflush(stdout);

    ////close(fd);

    return 0;
}
