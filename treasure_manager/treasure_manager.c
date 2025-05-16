#include "treasure_manager.h"
#include "io_functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>


#define BUF_SIZE 200


// -----------------------------------------------------------------------------
// helper functions
// -----------------------------------------------------------------------------

// writes treasure data to stdout ----------------------------------------------

void write_treasure_data(Treasure_t treasure){
    char buf[BUF_SIZE];

    write_message("Treasure ID: ");
    write_message(treasure.id);

    write_message(" -- User: ");
    write_message(treasure.user);

    write_message(" -- Latitude: ");
    sprintf(buf, "%.6f", treasure.latitude);
    write_message(buf);

    write_message(" -- Longitude: ");
    sprintf(buf, "%.6f", treasure.longitude);
    write_message(buf);

    write_message(" -- Clue: ");
    write_message(treasure.clue);

    write_message(" -- Value: ");
    sprintf(buf, "%d", treasure.value);
    write_message(buf);

    write_message("\n");
}


// reads treasure data from stdin ----------------------------------------------

Treasure_t read_treasure_data(Treasure_t treasure){
    char buf[BUF_SIZE];

    write_message("Enter treasure ID: ");
    read_input(treasure.id, ID_LENGTH);

    write_message("Enter username: ");
    read_input(treasure.user, USER_LENGTH);

    write_message("Enter latitude: ");
    read_input(buf, sizeof(buf));
    treasure.latitude = atof(buf);

    write_message("Enter longitude: ");
    read_input(buf, sizeof(buf));
    treasure.longitude = atof(buf);

    write_message("Enter clue: ");
    read_input(treasure.clue, CLUE_LENGTH);

    write_message("Enter value: ");
    read_input(buf, sizeof(buf));
    treasure.value = atoi(buf);

    return treasure;
}


// adds an action to the log file ----------------------------------------------

void log_action(char* hunt_path, char* log_msg){
    char log_filepath[BUF_SIZE];
    sprintf(log_filepath, "%s/logged_hunt.txt", hunt_path); // path to the log file

    int log_fd = open(log_filepath, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if(log_fd == -1){                                       // creates file if it doesn't exist 
        perror("failed to open log file\n");
        exit(-1);
    }

    if(write(log_fd, log_msg, strlen(log_msg)) == -1){      // writes log message to the file
        perror("failed to log message\n");
        close(log_fd);
        exit(-1);
    }

    if(close(log_fd) == -1){                                // closes the log file
        perror("failed to close file\n");
        exit(-1);
    }
}


// -----------------------------------------------------------------------------
// command functions
// -----------------------------------------------------------------------------

// adds a treasure to the hunt -------------------------------------------------

void add(char *hunt_id){

    struct stat buf;
    if(stat("Hunts", &buf) != 0){                                   // creates "Hunts" directory
        if(mkdir("Hunts", S_IRUSR | S_IWUSR | S_IXUSR) != 0){       // for storing all hunts
            perror("failed to create Hunts directory\n");
            exit(-1);
        }
    }

    char hunt_path[BUF_SIZE];
    sprintf(hunt_path, "Hunts/%s", hunt_id);                        // creates hunt path

    if(stat(hunt_path, &buf) != 0){                                 // creates hunt directory
        if(mkdir(hunt_path, S_IRUSR | S_IWUSR | S_IXUSR) != 0){     // if it doesn't exist
            perror("failed to create directory\n");
            exit(-1);
        }
        write_message("Hunt directory created successfully!\n");
    }
    else{
        write_message("Adding treasure to ");
        write_message(hunt_id);
        write_message("\n");
    }


    Treasure_t treasure;
    treasure = read_treasure_data(treasure);                        // reads treasure data

    char treasure_filepath[512];
    sprintf(treasure_filepath, "%s/treasures.bin", hunt_path);      // creates treasure file path

    int treasure_fd = open(treasure_filepath, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR); 
    if(treasure_fd == -1){                                          // opens / creates treasure file
        perror("failed to open treasures file\n");
        exit(-1);
    }

    if(write(treasure_fd, &treasure, sizeof(Treasure_t)) == -1){    // writes treasure data to file
        perror("failed to write treasure data\n");
        close(treasure_fd);
        exit(-1);
    }

    if(close(treasure_fd) == -1){                                   // closes the treasure file
        perror("failed to close file\n");
        exit(-1);
    }

    
    char log_msg[BUF_SIZE];                                         // logs the action
    sprintf(log_msg, "Added treasure %s\n", treasure.id);
    log_action(hunt_path, log_msg);

    char log_filepath[512];                                    // creates log file path
    char symlink_filepath[BUF_SIZE];                                // creates symlink file path
    sprintf(log_filepath, "%s/logged_hunt.txt", hunt_path);
    sprintf(symlink_filepath, "logged_hunt_%s", hunt_id);
    int symlink_fd = symlink(log_filepath, symlink_filepath);       // doesn't overwrite if it already exists

    write_message("Added treasure to ");
    write_message(hunt_id);
    write_message("!\n");
}


// lists all treasures in a hunt -----------------------------------------------

void list(char* hunt_id){
    
    char treasure_filepath[BUF_SIZE];                       // creates treasure file path
    snprintf(treasure_filepath, sizeof(treasure_filepath), "Hunts/%s/treasures.bin", hunt_id);

    int treasure_fd = open(treasure_filepath, O_RDONLY);    // opens treasure file
    if(treasure_fd == -1){
        perror("failed to open treasures file\n");
        exit(-1);
    }

    write_message("Hunt: ");                                // header
    write_message(hunt_id);
    write_message("\n");

    Treasure_t treasure;                                    // reads and prints each treasure
    int len;
    while((len = read(treasure_fd, &treasure, sizeof(Treasure_t))) > 0){
        if(len < sizeof(Treasure_t)){
            perror("error reading treasure data\n");
            exit(-1);
        }

        write_treasure_data(treasure);
    }

    struct stat info;                                      // gets file info
    if(stat(treasure_filepath, &info) == -1){
        perror("failed to get file info\n");
        close(treasure_fd);
        exit(-1);
    }

    char file_size[BUF_SIZE];
    sprintf(file_size, "File size: %ld bytes\n", info.st_size);
    write_message(file_size);

    write_message("Last modified: ");
    write_message(ctime(&info.st_mtime));
    write_message("\n");

    if(close(treasure_fd) == -1){                         // closes the treasure file
        perror("failed to close file\n");
        exit(-1);
    }
}


// views a treasure in a hunt --------------------------------------------------

void view(char* hunt_id, char* treasure_id){

    char treasure_filepath[BUF_SIZE];                    // creates treasure file path
    sprintf(treasure_filepath, "Hunts/%s/treasures.bin", hunt_id);

    int treasure_fd = open(treasure_filepath, O_RDONLY); // opens treasure file
    if(treasure_fd == -1){
        perror("failed to open treasures file\n");
        exit(-1);
    }

    Treasure_t treasure;
    int len;
    while((len = read(treasure_fd, &treasure, sizeof(Treasure_t))) > 0){

        if(strcmp(treasure.id, treasure_id) == 0){       // checks if treasure ID matches
            write_treasure_data(treasure);               // prints treasure data
            break;
        }
    }

    if(close(treasure_fd) == -1){
        perror("failed to close file\n");
        exit(-1);
    }

}


// removes a treasure from a hunt ----------------------------------------------

void remove_treasure(char* hunt_id, char* treasure_id){

    // writes all data to a temporary file, except the treasure to be removed

    char treasure_filepath[BUF_SIZE], aux_filepath[BUF_SIZE];
    sprintf(treasure_filepath, "Hunts/%s/treasures.bin", hunt_id);
    sprintf(aux_filepath, "Hunts/%s/aux.bin", hunt_id);

    int treasure_fd = open(treasure_filepath, O_RDONLY);
    if(treasure_fd == -1){
        perror("failed to open treasures file\n");
        exit(-1);
    }

    int aux_fd = open(aux_filepath, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IXUSR);
    if(aux_fd == -1){
        perror("failed to open temporary file\n");
        exit(-1);
    }

    Treasure_t treasure;
    int found = 0;
    while(read(treasure_fd, &treasure, sizeof(Treasure_t)) == sizeof(Treasure_t)){
        if(strcmp(treasure.id, treasure_id) == 0){
            found = 1;                                       // if treasure ID matches
            continue;                                        // skip writing to the temporary file
        }
        if(write(aux_fd, &treasure, sizeof(Treasure_t)) == -1){
            perror("failed to write in temporary file\n");
            close(treasure_fd);
            close(aux_fd);
            exit(-1);
        }
    }

    close(treasure_fd);
    close(aux_fd);

    if(found == 0){
        write_message("Treasure not found\n");              // message if treasure not found
        remove(aux_filepath);
        return;
    }

    // replaces the original treasures file with the temporary file

    if(remove(treasure_filepath) == -1){
        perror("failed to remove original treasures file\n");
        exit(-1);
    }

    if(rename(aux_filepath, treasure_filepath) == -1){
        perror("failed to rename temporary file\n");
        exit(-1);
    }

    write_message("Removed treasure ");
    write_message(treasure_id);
    write_message(" from hunt ");
    write_message(hunt_id);
    write_message("\n");

    char log_msg[BUF_SIZE];                                 // logs the action
    sprintf(log_msg, "Removed treasure %s\n", treasure_id);
    char hunt_path[BUF_SIZE];
    sprintf(hunt_path, "Hunts/%s", hunt_id);
    log_action(hunt_path, log_msg);
}


// removes a hunt and all its treasures -------------------------------------

void remove_hunt(char* hunt_id){

    char hunt_path[BUF_SIZE];                                       // creates hunt path
    sprintf(hunt_path, "Hunts/%s", hunt_id);
    DIR* dir = opendir(hunt_path);
    if(dir == NULL){
        perror("failed to open directory\n");
        exit(-1);
    }

    struct dirent* f;
    char filepath[512];

    while((f = readdir(dir)) != NULL){
        if(strcmp(f->d_name, ".") == 0 || strcmp(f->d_name, "..") == 0){
            continue;                                              // skip . and .. 
        }

        sprintf(filepath, "%s/%s", hunt_path, f->d_name);
        if(remove(filepath) == -1){                                // removes each file in the hunt directory
            perror("failed to remove file in hunt\n");
            closedir(dir);
            exit(-1);
        }
    }

    if(closedir(dir) == -1){
        perror("failed to close directory\n");
        exit(-1);
    }


    if(rmdir(hunt_path) == -1){                                      // removes the hunt directory
        perror("failed to remove hunt directory\n");
        exit(-1);
    }
    
    
    char symlink_filepath[BUF_SIZE];                               // removes the symbolic link       
    sprintf(symlink_filepath, "logged_hunt_%s", hunt_id);
    if(remove(symlink_filepath) == -1){
        perror("failed to remove symbolic link\n");
        exit(-1);
    }

    write_message("Removed ");
    write_message(hunt_id);
    write_message("\n");
}


// lists all hunts in the "Hunts" directory ------------------------------------

void list_hunts(){
    DIR* dir = opendir("Hunts");                  // opens the "Hunts" directory
    if(dir == NULL){
        perror("failed to open Hunts directory\n");
        exit(-1);
    }

    struct dirent* current_dir;
    struct stat path_stat;
    char path[512];
    char treasure_path[600];

    write_message("Hunts:\n");

    while((current_dir = readdir(dir)) != NULL){
        if(strcmp(current_dir->d_name, ".") == 0 || strcmp(current_dir->d_name, "..") == 0){
            continue;                            // skip . and ..
        }

        sprintf(path, "Hunts/%s", current_dir->d_name);

        if (stat(path, &path_stat) == -1) {
            perror("stat failed\n");
            exit(-1);
        }
        if(S_ISDIR(path_stat.st_mode) ){         // if it's a directory
            
            sprintf(treasure_path, "%s/treasures.bin", path);
            int treasure_fd = open(treasure_path, O_RDONLY);
            if(treasure_fd == -1){
                perror("failed to open treasures file\n");
                exit(-1);
            }
            int count = 0;
            Treasure_t treasure;
            if(treasure_fd != -1){
                while(read(treasure_fd, &treasure, sizeof(Treasure_t)) == sizeof(Treasure_t)){
                    count++;
                }
                close(treasure_fd);
            }
            
            write_message("\t");
            write_message(current_dir->d_name);    // prints the directory name
            write_message(" -- ");
            char buf[BUF_SIZE];
            sprintf(buf, "%d treasures\n", count); // prints the number of treasures
            write_message(buf);
        }
    }

    if(closedir(dir) == -1){
        perror("failed to close directory\n");
        exit(-1);
    }
}