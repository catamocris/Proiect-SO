#define _XOPEN_SOURCE 700   // for sigaction

#include "treasure_hub.h"
#include "../treasure_manager/io_functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>


#define EXEC_PATH "../treasure_manager/treasure_manager" // path to treasure_manager executable


pid_t monitor_pid = -1;     // variables used to track the monitor process
int monitor_running = 0;
int monitor_stopping = 0;


int monitor_pipe[2];         // pipe: main process reads, monitor process writes


// -----------------------------------------------------------------------------
// hub functions using treasure_manager
// -----------------------------------------------------------------------------

// lists all hunts -------------------------------------------------------------

void list_hunts_hub(){
    if(monitor_stopping == 1){
        write_message("Monitor is stopping! Please wait...\n");
        return;
    }
    if(monitor_running == 0){
        write_message("Monitor not running!\n");
        return;
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("Failed to fork process\n");
        exit(-1);
    }
    if(pid == 0) {              // child process -> executes treasure_manager with --list_hunts
        if (execlp(EXEC_PATH, EXEC_PATH, "--list_hunts", NULL) == -1) {
            perror("Error executing treasure_manager\n");
            exit(-1);
        }
    } else {
        waitpid(pid, NULL, 0);  // parent process -> waits for child to finish
    }
}

// lists all treasures for a specific hunt -------------------------------------

void list_treasures_hub(){
    if(monitor_stopping == 1){
        write_message("Monitor is stopping! Please wait...\n");
        return;
    }
    if(monitor_running == 0){
        write_message("Monitor not running!\n");
        return;
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("Failed to fork process\n");
        exit(-1);
    }
    if(pid == 0){               // child process -> executes treasure_manager with --list
        char hunt_id[10];
        write_message("Enter hunt_id: ");       // prompt for hunt_id
        read_input(hunt_id, sizeof(hunt_id));
        
        if (execlp(EXEC_PATH, EXEC_PATH, "--list", hunt_id, NULL) == -1) {
                perror("Error executing treasure_manager\n");
                exit(-1);
        }
    } else {
        waitpid(pid, NULL, 0);  // parent process -> waits for child to finish
    }
}

// views a specific treasure from a specific hunt ------------------------------

void view_treasure_hub(){
    if(monitor_stopping == 1){
        write_message("Monitor is stopping! Please wait...\n");
        return;
    }
    if(monitor_running == 0){
        write_message("Monitor not running!\n");
        return;
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("Failed to fork process.\n");
        exit(-1);
    }
    
    if(pid == 0){               // child process -> executes treasure_manager with --view
        char hunt_id[10];
        char treasure_id[10];

        write_message("Enter hunt_id: ");           // prompt for hunt_id
        read_input(hunt_id, sizeof(hunt_id));

        write_message("Enter treasure_id: ");       // prompt for treasure_id
        read_input(treasure_id, sizeof(treasure_id));
        
        if (execlp(EXEC_PATH, EXEC_PATH, "--view", hunt_id, treasure_id, NULL) == -1) {
                perror("Error executing treasure_manager\n");
                exit(-1);
        }
    } else {
        waitpid(pid, NULL, 0);  // parent process -> waits for child to finish
    }
}


// -----------------------------------------------------------------------------
// monitor functions
// -----------------------------------------------------------------------------

// handles signals sent to the monitor process ---------------------------------

void handle_signal(int sig){
    if(sig == SIGUSR1){
        list_hunts_hub();
    } else if(sig == SIGUSR2){
        list_treasures_hub();
    } else if(sig == SIGINT){
        view_treasure_hub();
    }
}

void handle_sigterm(int sig){
    sleep(5);                               // delayes for 5 seconds before stopping
    write_message("Monitor stopped!\n");
    exit(0);                                // exits the monitor process
}

void handle_sigchld(int sig){               // sends SIGCHLD to parent when a child process terminates
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG); 
    if(pid == monitor_pid){                 // resets flags if the monitor process terminates
        monitor_running = 0;
        monitor_stopping = 0;
        monitor_pid = -1;
    }
}

// runs the monitor process ----------------------------------------------------

void run_monitor(){
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handle_signal;          // sets the signal handler

    sigaction(SIGUSR1, &sa, NULL);          // handle SIGUSR1
    sigaction(SIGUSR2, &sa, NULL);          // handle SIGUSR2
    sigaction(SIGINT, &sa, NULL);           // handle SIGINT

    struct sigaction sa_term;
    memset(&sa_term, 0, sizeof(sa_term));
    sa_term.sa_handler = handle_sigterm;    // sets the signal handler for SIGTERM
    sigaction(SIGTERM, &sa_term, NULL);

    while(1){
        pause();                            // waits for signals
    }
}

// starts the monitor process --------------------------------------------------

void start_monitor(){
    if(monitor_pid != -1){
        write_message("Monitor already running!\n");
        return;
    }

    if(pipe(monitor_pipe) == -1){           // creates the pipe
        perror("Failed to create pipe\n");
        exit(-1);
    }
    
    struct sigaction sa_child;
    memset(&sa_child, 0, sizeof(struct sigaction));
    sa_child.sa_handler = handle_sigchld;   // sets the signal handler for SIGCHLD
    sigaction(SIGCHLD, &sa_child, NULL);

    monitor_pid = fork();
    if(monitor_pid < 0){
        perror("Failed to fork monitor process\n");
        exit(-1);
    }

    if(monitor_pid == 0){                   // child process -> runs the monitor
        close(monitor_pipe[0]);                 // closes read
        dup2(monitor_pipe[1], STDOUT_FILENO);   // redirects stdout to pipe
        close(monitor_pipe[1]);                 // closes write
        run_monitor();
        exit(0);
    } else {                                // parent process
        close(monitor_pipe[1]);                 // closes write
        monitor_running = 1;
        monitor_stopping = 0;
        write_message("Monitor started successfully!\n");
    }
}

// stops the monitor process ---------------------------------------------------

void stop_monitor(){
    if(monitor_pid == -1){
        write_message("No monitor process running!\n");
        return;
    }
    if(monitor_stopping == 1){
        write_message("Monitor is stopping! Please wait...\n");
        return;
    }
    monitor_stopping = 1;
    write_message("Stopping monitor...\n");
    kill(monitor_pid, SIGTERM);         // sends SIGTERM to the monitor process

    char buffer[1024];
    int bytes_read = read(monitor_pipe[0], buffer, sizeof(buffer)); // reads from the pipe
    if(bytes_read > 0){
        buffer[bytes_read] = '\0';
        write_message(buffer);         // prints the message from the monitor process
    }
    close(monitor_pipe[0]);            // closes read
}

// exits the program -----------------------------------------------------------

void exit_command(){
    if(monitor_stopping == 1){
        write_message("Monitor is stopping! Please wait...\n");
        return;
    }
    if(monitor_running != 0){
        write_message("Monitor is still running! Please stop before exiting.\n");
        return;
    }
    write_message("Exiting...\n");
    exit(0);
}
