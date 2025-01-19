#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
 
// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)



void* threadfunc(void* thread_param)
{
    
    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    printf("threadfunc...0...");
    fflush(stdout);

    usleep( (thread_func_args->wait_to_obtain_ms)*1000);

    printf("threadfunc...10...");
    fflush(stdout);

    pthread_mutex_lock(thread_func_args->mutex);

    printf("threadfunc...20...");
    fflush(stdout);

    usleep( (thread_func_args->wait_to_release_ms)*1000);

    printf("threadfunc...30...");
    fflush(stdout);

    thread_func_args->thread_complete_success = true;

    pthread_mutex_unlock(thread_func_args->mutex);

    printf("threadfunc...40...");
    fflush(stdout);

    //free(thread_func_args);

    return thread_param;
}

/*
** threadfunc: responsible to free thread_func_args
*/
bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
     
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    struct thread_data *thread_func_args = malloc(sizeof(struct thread_data));

    if (thread_func_args==NULL) {
        perror("ERROR: allocate memory for thread_data");
        return false;
    }

    thread_func_args->mutex = mutex;
    thread_func_args->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_func_args->wait_to_release_ms = wait_to_release_ms;
    thread_func_args->thread_complete_success = false;
    
    printf("test...0...\n");
    fflush(stdout);

    int create_rc = pthread_create(thread, NULL, 
                threadfunc, thread_func_args);
    if (create_rc != 0) {
        perror("ERROR: failed to create thread\n");
        free(thread_func_args);
        return false;
    }

    printf("wait_to_obtain_ms==%d\n", wait_to_obtain_ms);
    printf("wait_to_release_ms==%d\n", wait_to_release_ms);
    fflush(stdout);


    /* 
    usleep( (thread_func_args->wait_to_release_ms)*1100);

    printf("test...10...\n");
    fflush(stdout);
    //struct thread_data *thread_result;
    int join_rc = pthread_join(*thread, NULL);
    printf("test...20...\n");
    fflush(stdout);
    if (join_rc != 0) {
        perror("ERROR: failed to join thread\n\n");
        free(thread_func_args);
        return false;
    } 
    

    printf("test...30...\n");
    fflush(stdout);

    bool complete_status = thread_func_args->thread_complete_success;
    printf("complete_status==%d", complete_status);
    free(thread_func_args);
    printf("test...10...");
    fflush(stdout);
  
    
    return complete_status;
    */
    return true;
}

