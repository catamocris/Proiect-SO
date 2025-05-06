#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>


pid_t monitor_pid = -1;
int monitor_running = 0;


void list_hunts(){
    printf("listing hunts...\n");
}

void list_treasures(){
    char hunt_id[10];
    write(1, "Enter hunt_id: ", strlen("Enter hunt_id: "));
    int bytes_read = read(0, hunt_id, sizeof(hunt_id)-1);
    write(0, "\n", strlen("\n"));
    if(bytes_read < 0){
        perror("Could not read hunt_id.");
        exit(-1);
    }

    hunt_id[bytes_read] = '\0';
    if(hunt_id[bytes_read - 1] == '\n')
        hunt_id[bytes_read - 1] = '\0';
    
    if (execlp("./treasure_manager", "./treasure_manager", "--list", hunt_id, (char *)NULL) == -1) {
            perror("Error executing treasure_manager");
    }
    
}

void view_treasure(){
    printf("viewing treasure...\n");
}


void handle_signal(int sig){
    if(sig == SIGUSR1){
        list_hunts();
    } else if(sig == SIGUSR2){
        list_treasures();
    } else if(sig == SIGINT){
        view_treasure();
    }
}


void run_monitor(){
    // handle_signal pt list_hunts, list_treasures sau view_treasures
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    memset(&sa, 0, sizeof(struct sigaction));

    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    // seteaza SIGTERM la valoarea default
    // daca monitorul primeste SIGTERM, opreste imediat
    struct sigaction sa_term;
    memset(&sa_term, 0, sizeof(sa_term));
    sa_term.sa_handler = SIG_DFL;
    sigaction(SIGTERM, &sa_term, NULL);

    while(1){
        pause();      // asteapta semnale
    }
}


void start_monitor(){
    // daca monitorul e deja running, afisam mesaj si inchidem apelul functiei
    if(monitor_pid != -1){
        write(1, "Monitor already running. \n", strlen("Monitor already running. \n"));
        return;
    }

    // altfel fork
    monitor_pid = fork();
    if(monitor_pid < 0){
        perror("Failed to fork monitor process.");
        exit(-1);
    }
    if(monitor_pid == 0){   // copil
        run_monitor();
        exit(0);
    } else{                 // parinte
        monitor_running = 1;
        write(1, "Monitor started successfully. \n", strlen("Monitor started successfully. \n"));
    }
}


void stop_monitor(){
    // daca nu e niciun proces, inchidem apelul functiei
    if(monitor_pid == -1){
        write(1, "No monitor process running. \n", strlen("No monitor process running. \n"));
        return;
    }
    kill(monitor_pid, SIGTERM);     // trimite semnal pt terminarea procesului
    waitpid(monitor_pid, NULL, 0);  // asteapta sa se termine monitorul
    monitor_pid = -1;
    monitor_running = 0;
    write(1, "Monitor stopped. \n", strlen("Monitor stopped. \n"));
}


void exit_command(){
    // daca monitorul e inca running, afisam mesaj si inchidem apelul functiei
    if(monitor_running != 0){
        write(1, "Can't exit while monitor is running. \n", strlen("Can't exit while monitor is running. \n"));
        return;
    }
    write(1, "Exiting ... \n", strlen("Exiting ... \n"));
    exit(0);
}


int main(){

    char input[256];
    int bytes_read;

    // creez executabil pt treasure_manager
    char *exec_command = "gcc -o treasure_manager treasure_manager.c";
    int check_exec = system(exec_command); 
    if (check_exec) 
    {
        perror("Could not compile treasure_manager");
        exit(-1);
    }

    while(1){

        write(1, ">> ", strlen(">> "));                   // 1 - stdout
        bytes_read = read(0, input, sizeof(input)-1);     // 0 - stdin
        if(bytes_read < 0){
            perror("Could not read input.");
            exit(-1);
        }

        input[bytes_read] = '\0';
        if(input[bytes_read - 1] == '\n')
            input[bytes_read - 1] = '\0';

        if(strcmp(input, "start_monitor") == 0){
            start_monitor();
        } else if(strcmp(input, "list_hunts") == 0){
            // list hunts
        } else if(strcmp(input, "list_treasures") == 0){
            list_treasures();
        } else if(strcmp(input, "view_treasures") == 0){
            // view treasure
        } else if(strcmp(input, "stop_monitor") == 0){
            stop_monitor();
        } else if(strcmp(input, "exit") == 0){
            exit_command();
        } else{
            write(1, "Unknown command. \n", strlen("Unknown command. \n"));
        }
    }

    return 0;
}