#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>

#define PORT 9000
#define BUFFER_SIZE 1024
#define FILE_PATH "/var/tmp/aesdsocketdata"
volatile sig_atomic_t is_server_running = 1;

int server_socket = -1;
FILE *file = NULL;


void handle_signal(int sig) {

    if (sig==SIGINT || sig==SIGTERM) {
        
        syslog(LOG_INFO, "Caught signal, exiting");
        
        //
        is_server_running = 0;

        if (file != NULL) {
            fclose(file);
        }

        if (server_socket != -1) {
            close(server_socket);
        }

        remove(FILE_PATH);
        exit(0);
    }
}

int bind_server_socket() {

     struct sockaddr_in server_addr;

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1) {
        syslog(LOG_ERR, "Failed to create socket");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        syslog(LOG_ERR, "Failed to bind socket to port %d\n", PORT);
        close(server_socket);
        return -1;
    }

    return server_socket;
}


char* string_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    // Calculate the required size
    int size = vsnprintf(NULL, 0, format, args) + 1; // +1 for null terminator
    va_end(args);

    if (size <= 0) {
        return NULL; // Failed to calculate size
    }

    // Allocate memory for the string
    char* result = malloc(size);
    if (!result) {
        return NULL; // Allocation failed
    }

    // Format the string
    va_start(args, format);
    vsnprintf(result, size, format, args);
    va_end(args);

    return result; // Caller must free the memory
}


void log_info(char * msg) {

    printf("%s\n", msg);

    openlog("INFO_LOG", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "%s\n", msg);
    closelog();
}   

void log_error(char * msg) {

    printf("%s\n", msg);

    openlog("ERR_LOG", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_ERR, "%s\n", msg);
    closelog();
}   

void send_file_to_client_socket(int client_socket, FILE *file) {

    char buffer[BUFFER_SIZE];
     
    fseek(file, 0, SEEK_SET);
    while(1) {
            
        int file_bytes_read=fread(buffer, 1, BUFFER_SIZE, file);
        if (file_bytes_read <= 0) {
            break;
        } else {
            buffer[file_bytes_read] = '\0';
        }
                 
        size_t start = 0;
        for (size_t i=0; i < file_bytes_read; ++i) {
            if (buffer[i] == '\n') {
                   
                send(client_socket, buffer+start, i-start, 0);
                send(client_socket, "\n", strlen("\n"), 0);

                start = i+1;
            }
        }
               

        // send remaining stuff
        if (start < file_bytes_read) {
            fwrite(buffer+start, 1, file_bytes_read-start, file);

            for (size_t i=0; i < file_bytes_read-start; ++i) {
                if ((buffer+start)[i] == '\n') {
                    send(client_socket, buffer+start, i-start, 0);  
                    start = i+1;
                }
            }
        }
    }
}

int socket_main_loop(char *tag) {

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    //
    struct sockaddr_in client_addr;

    int client_socket;
    socklen_t client_len;

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
 

    server_socket = bind_server_socket();

    log_info(string_printf("TAG==%s\n", tag));

    if (listen(server_socket, 5) == -1) {
        log_error("Failed to listen on socket");
        close(server_socket);
        return -1;
    }

    file = fopen(FILE_PATH, "w+");
    if (file==NULL) {  
        log_error(string_printf("Failed to open file %s\n", FILE_PATH));    
        close(server_socket);
        return -1;
    }

    // accept client socket connection loop
    while(1) {

        client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

        if (client_socket == -1) {
            log_error(string_printf("Failed to accept connection\n"));
            break;
        }
 
        log_info(string_printf("Accepted connection from %s\n", inet_ntoa(client_addr.sin_addr)));

        //
        send_file_to_client_socket(client_socket, file);


        // recv client packet loop after connected
        while (1) {

            int bytes_peek = recv(client_socket, buffer, BUFFER_SIZE, MSG_PEEK);

            int bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_read <= 0) {
                break;
            } else {
                buffer[bytes_read] = '\0';
            }


            log_info(string_printf("DEBUG::buffer==%s\n\n", buffer));

            int write_rc = fwrite(buffer, 1, bytes_read, file);

            send(client_socket, buffer, strlen(buffer), 0);
             
        
        }

        close(client_socket);

    }

    //
    fclose(file);
    close(server_socket);

    /* 
    while(is_server_running==1) {
        log_info("socket loop...\n");
        sleep(1);
    }
    */

    return 0;
}

void redirect_std_to_null() {
    int null_fd = open("/dev/null", O_RDWR);
    if (null_fd < 0) {
        perror("ERROR: open /dev/null failed\n");
        exit(1);
    }

    if (dup2(null_fd, STDIN_FILENO) < 0) {
        perror("dup2 stdin FAILED\n");
        exit(1);
    }
    if (dup2(null_fd, STDOUT_FILENO) < 0) {
        perror("dup2 stdout FAILED\n");
        exit(1);
    }
    if (dup2(null_fd, STDERR_FILENO) < 0) {
        perror("dup2 stderr FAILED\n");
        exit(1);
    }
}

void run_as_daemon() {

    pid_t pid = fork();

    if (pid==0) {
        if (setsid() < 0) {
            perror("ERROR: setsid");
            exit(1);
        }
        signal(SIGHUP, SIG_IGN);

        //
        if (chdir("/") < 0) {
            perror("ERROR: chdir failed\n");
            exit(1);
        }



        socket_main_loop("RUN_AS_DAEMON");


    } else if (pid > 0) {
        // parent process exit
        exit(0);

    } else {
        perror("ERROR: fork failed\n");
        exit(1);
    }

}

int main(int argc, char *argv[]) {

    int daemon_mode = 0;
    int opt; 

    int socket_id = bind_server_socket();
    if (socket_id < 0) {
        perror("ERROR: bind server port failed\n");
        exit(1);
    } else {
        // close server port after test
        close(socket_id);
    }

    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
            case 'd':
                daemon_mode = 1;
                break;
            default:
                break;
        }
    }

    if (daemon_mode) {
        run_as_daemon();
    } else {
        socket_main_loop("RUN_AS_PARENT");
    }

    return 0;
}