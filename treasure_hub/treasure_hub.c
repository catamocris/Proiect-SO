#define _XOPEN_SOURCE 700

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


#define EXEC_PATH "../treasure_manager/treasure_manager"

pid_t monitor_pid = -1;
int monitor_running = 0;


void list_hunts_hub(){
    if(monitor_running == 0){
        write_message("Monitor not running!\n");
        return;
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("Failed to fork process\n");
        exit(-1);
    }
    if(pid == 0) { 
        if (execlp(EXEC_PATH, EXEC_PATH, "--list_hunts", NULL) == -1) {
            perror("Error executing treasure_manager\n");
            exit(-1);
        }
    } else {
        waitpid(pid, NULL, 0);
    }
    write_message("\n");
}

void list_treasures_hub(){
    if(monitor_running == 0){
        write_message("Monitor not running!\n");
        return;
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("Failed to fork process\n");
        exit(-1);
    }
    if(pid == 0){
        char hunt_id[10];
        write_message("Enter hunt_id: ");
        read_input(hunt_id, sizeof(hunt_id));
        
        if (execlp(EXEC_PATH, EXEC_PATH, "--list", hunt_id, NULL) == -1) {
                perror("Error executing treasure_manager\n");
                exit(-1);
        }
    } else {
        waitpid(pid, NULL, 0);
    }
    write_message("\n");
}

void view_treasure_hub(){
    if(monitor_running == 0){
        write_message("Monitor not running!\n");
        return;
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("Failed to fork process.\n");
        exit(-1);
    }
    
    if(pid == 0){
        char hunt_id[10];
        char treasure_id[10];

        write_message("Enter hunt_id: ");
        read_input(hunt_id, sizeof(hunt_id));

        write_message("Enter treasure_id: ");
        read_input(treasure_id, sizeof(treasure_id));
        
        if (execlp(EXEC_PATH, EXEC_PATH, "--view", hunt_id, treasure_id, NULL) == -1) {
                perror("Error executing treasure_manager\n");
                exit(-1);
        }
    } else {
        waitpid(pid, NULL, 0);
    }
    write_message("\n");
}

void handle_signal(int sig){
    if(sig == SIGUSR1){
        list_hunts_hub();
    } else if(sig == SIGUSR2){
        list_treasures_hub();
    } else if(sig == SIGINT){
        view_treasure_hub();
    }
}
void run_monitor(){
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    memset(&sa, 0, sizeof(struct sigaction));

    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    struct sigaction sa_term;
    memset(&sa_term, 0, sizeof(sa_term));
    sa_term.sa_handler = SIG_DFL;
    sigaction(SIGTERM, &sa_term, NULL);

    while(1){
        pause();
    }
}

void start_monitor(){
    if(monitor_pid != -1){
        write_message("Monitor already running!\n");
        return;
    }
    monitor_pid = fork();
    if(monitor_pid < 0){
        perror("Failed to fork monitor process\n");
        exit(-1);
    }
    if(monitor_pid == 0){
        run_monitor();
        exit(0);
    } else {
        monitor_running = 1;
        write_message("Monitor started successfully!\n");
    }
}

void stop_monitor(){
    if(monitor_pid == -1){
        write_message("No monitor process running!\n");
        return;
    }
    kill(monitor_pid, SIGTERM);
    waitpid(monitor_pid, NULL, 0);
    monitor_pid = -1;
    monitor_running = 0;
    write_message("Monitor stopped!\n");
}

void exit_command(){
    if(monitor_running != 0){
        write_message("Monitor is still running! Please stop before exiting.\n");
        return;
    }
    write_message("Exiting...\n");
    exit(0);
}
