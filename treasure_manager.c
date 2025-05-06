#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define ID_LENGTH 15
#define USER_LENGTH 15
#define COORDINATE_LENGTH 10
#define CLUE_LENGTH 100
#define VALUE_LENGTH 100
#define FP_LENGTH 50

#define BUF_SIZE 256

typedef struct{
    char id[ID_LENGTH];
    char user[USER_LENGTH];
    char latitude[COORDINATE_LENGTH];
    char longitude[COORDINATE_LENGTH];
    char clue[CLUE_LENGTH];
    char value[VALUE_LENGTH];
}Treasure_t;


void write_message(char *msg){
    write(1, msg, strlen(msg));
}


void read_input(char *input, int size){
    int bytes_read = read(0, input, size-1);
    
    if(bytes_read > 0){
        input[bytes_read] = '\0';
        if (input[bytes_read - 1] == '\n') {
            input[bytes_read - 1] = '\0';
        }
    }
    
}


void write_treasure_data(Treasure_t treasure){
        write_message("Treasure ID: ");
        write_message(treasure.id);
        write_message(" -- User: ");
        write_message(treasure.user);
        write_message(" -- Latitude: ");
        write_message(treasure.latitude);
        write_message(" -- Longitude: ");
        write_message(treasure.longitude);
        write_message(" -- Clue: ");
        write_message(treasure.clue);
        write_message(" -- Value: ");
        write_message(treasure.value);
        write_message("\n");
}


void log_action(char* hunt_id, char* log_msg){
    char log_filepath[100];
    snprintf(log_filepath, sizeof(log_filepath), "%s/logged_hunt.bin", hunt_id);
    int log_fd = open(log_filepath, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IXUSR);
    if(log_fd == -1){
        perror("failed to open log file");
        exit(-1);
    }

    if(write(log_fd, log_msg, strlen(log_msg)) == -1){
        perror("failed to log message");
        close(log_fd);
        exit(-1);
    }

    if(close(log_fd) == -1){
        perror("failed to close file");
        exit(-1);
    }
}


void add(char *hunt_id){

    // verific daca exista directorul hunt, creez daca nu exista
    struct stat buf;
    if(stat(hunt_id, &buf) != 0){
        if(mkdir(hunt_id, S_IRUSR | S_IWUSR | S_IXUSR) != 0){
            perror("failed to create directory");
            exit(-1);
        }
        write_message("Hunt directory created successfully !\n");
    }
    else{
        write_message("Adding treasure to ");
        write_message(hunt_id);
        write_message("\n");
    }

    // deschid directorul
    DIR *dir = opendir(hunt_id);
    if(dir == NULL){
        perror("failed to open directory");
        exit(-1);
    }

    // inchid directorul
    if(closedir(dir) == -1){
        perror("failed to close directory");
        exit(-1);
    }

    // citesc datele pt treasure
    Treasure_t treasure;

    write_message("Enter treasure ID: ");
    read_input(treasure.id, ID_LENGTH);

    write_message("Enter username: ");
    read_input(treasure.user, USER_LENGTH);

    write_message("Enter latitude: ");
    read_input(treasure.latitude, COORDINATE_LENGTH);

    write_message("Enter longitude: ");
    read_input(treasure.longitude, COORDINATE_LENGTH);

    write_message("Enter clue: ");
    read_input(treasure.clue, CLUE_LENGTH);

    write_message("Enter value: ");
    read_input(treasure.value, VALUE_LENGTH);

    // scriu datele in fisierul treasures
    char treasure_filepath[100];
    snprintf(treasure_filepath, sizeof(treasure_filepath), "%s/treasures.bin", hunt_id);
    int treasure_fd = open(treasure_filepath, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IXUSR); 
    if(treasure_fd == -1){
        perror("failed to open treasures file");
        exit(-1);
    }

    if(write(treasure_fd, &treasure, sizeof(Treasure_t)) == -1){
        perror("failed to write treasure data");
        close(treasure_fd);
        exit(-1);
    }

    if(close(treasure_fd) == -1){
        perror("failed to close file");
        exit(-1);
    }

    // dau log la operatia facuta
    char log_msg[100];
    snprintf(log_msg, sizeof(log_msg), "Added treasure %s\n", treasure.id);
    log_action(hunt_id, log_msg);

    // creez legatura simbolica
    char log_filepath[100];
    char symlink_filepath[100];
    snprintf(log_filepath, sizeof(log_filepath), "%s/logged_hunt.bin", hunt_id);
    snprintf(symlink_filepath, sizeof(symlink_filepath), "logged_hunt_%s", hunt_id);
    int symlink_fd = symlink(log_filepath, symlink_filepath); // daca exista deja, nu se face overwrite

    write_message("Added treasure to ");
    write_message(hunt_id);
    write_message("!\n");

}

void list(char* hunt_id){
    
    // fac filepath ul pt a putea deschide fisierul
    char treasure_filepath[100];
    snprintf(treasure_filepath, sizeof(treasure_filepath), "%s/treasures.bin", hunt_id);

    // deschid fisierul
    int treasure_fd = open(treasure_filepath, O_RDONLY);
    if(treasure_fd == -1){
        perror("failed to open treasures file");
        exit(-1);
    }

    // afisez
    write_message("Hunt: ");
    write_message(hunt_id);
    write_message("\n");

    Treasure_t treasure;
    int len;
    while(len = read(treasure_fd, &treasure, sizeof(Treasure_t))){
        if(len < sizeof(Treasure_t)){
            perror("error reading treasure data");
            exit(-1);
        }

        write_treasure_data(treasure);
    }

    // file info 
    struct stat info;
    if(stat(treasure_filepath, &info) == -1){
        perror("failed to get file info");
        close(treasure_fd);
        exit(-1);
    }

    char file_size[BUF_SIZE];
    snprintf(file_size, sizeof(file_size), "File size: %ld bytes\n", info.st_size);
    write_message(file_size);

    write_message("Last modified: ");
    write_message(ctime(&info.st_mtime));

    // inchid fisierul
    if(close(treasure_fd) == -1){
        perror("failed to close file");
        exit(-1);
    }
}

void view(char* hunt_id, char* treasure_id){
    
    char treasure_filepath[100];
    snprintf(treasure_filepath, sizeof(treasure_filepath), "%s/treasures.bin", hunt_id);

    int treasure_fd = open(treasure_filepath, O_RDONLY);
    if(treasure_fd == -1){
        perror("failed to open treasures file");
        exit(-1);
    }

    Treasure_t treasure;
    int len;
    
    while((len = read(treasure_fd, &treasure, sizeof(Treasure_t))) > 0){

        // cand gasesc id ul, afisez datele
        if(strcmp(treasure.id, treasure_id) == 0){
            write_treasure_data(treasure);
            break;
        }
    }

    if(close(treasure_fd) == -1){
        perror("failed to close file");
        exit(-1);
    }

}

void remove_treasure(char* hunt_id, char* treasure_id){

    // scriu datele de la treasures intr-un fisier auxiliar, mai putin de la treasure ul care trebuie sters
    char treasure_filepath[100], aux_filepath[100];
    snprintf(treasure_filepath, sizeof(treasure_filepath), "%s/treasures.bin", hunt_id);
    snprintf(aux_filepath, sizeof(aux_filepath), "%s/aux.bin", hunt_id);

    int treasure_fd = open(treasure_filepath, O_RDONLY);
    if(treasure_fd == -1){
        perror("failed to open treasures file");
        exit(-1);
    }

    int aux_fd = open(aux_filepath, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IXUSR);
    if(aux_fd == -1){
        perror("failed to open temporary file");
        exit(-1);
    }

    Treasure_t treasure;
    int found = 0;
    while(read(treasure_fd, &treasure, sizeof(Treasure_t)) == sizeof(Treasure_t)){
        if(strcmp(treasure.id, treasure_id) == 0){
            found = 1;
            continue;   // daca gasim treasure ul, sarim peste
        }
        if(write(aux_fd, &treasure, sizeof(Treasure_t)) == -1){
            perror("failed to write in temporary file");
            close(treasure_fd);
            close(aux_fd);
            exit(-1);
        }
    }

    close(treasure_fd);
    close(aux_fd);

    if(found == 0){
        write_message("treasure not found \n");
        remove(aux_filepath);
        return;
    }

    // inlocuiesc fisierul treasures original cu cel auxiliar
    if(remove(treasure_filepath) == -1){
        perror("failed to remove original treasures file");
        exit(-1);
    }

    if(rename(aux_filepath, treasure_filepath) == -1){
        perror("failed to rename temporary file");
        exit(-1);
    }

    write_message("Removed treasure ");
    write_message(treasure_id);
    write_message(" from hunt ");
    write_message(hunt_id);
    write_message("\n");

    // log la stergerea treasure ului
    char log_msg[100];
    snprintf(log_msg, sizeof(log_msg), "Removed treasure %s\n", treasure_id);
    log_action(hunt_id, log_msg);
}

void remove_hunt(char* hunt_id){

    // merg in director si sterg toate fisierele
    DIR* dir = opendir(hunt_id);
    if(dir == NULL){
        perror("failed to open directory");
        exit(-1);
    }

    struct dirent* f;
    char filepath[300];

    while((f = readdir(dir)) != NULL){
        // skip . (leg director curent) si .. (leg director parinte)
        if(strcmp(f->d_name, ".") == 0 || strcmp(f->d_name, "..") == 0){
            continue;
        }

        snprintf(filepath, sizeof(filepath), "%s/%s", hunt_id, f->d_name);
        if(remove(filepath) == -1){
            perror("failed to remove file in hunt");
            closedir(dir);
            exit(-1);
        }
    }

    if(closedir(dir) == -1){
        perror("failed to close directory");
        exit(-1);
    }

    // sterg directorul
    if(rmdir(hunt_id) == -1){
        perror("failed to remove hunt directory");
        exit(-1);
    }
    
    // sterg legatura simbolica
    char symlink_filepath[100];
    snprintf(symlink_filepath, sizeof(symlink_filepath), "logged_hunt_%s", hunt_id);
    if(remove(symlink_filepath) == -1){
        perror("failed to remove symbolic link");
        exit(-1);
    }

    write_message("removed ");
    write_message(hunt_id);
    write_message("\n");
}


void list_hunts(){
    DIR* dir = opendir(".");
    if(dir == NULL){
        perror("failed to open current directory");
        exit(-1);
    }

    struct dirent* current_dir;
    struct stat path_stat;

    write_message("Hunts:\n");

    while((current_dir = readdir(dir)) != NULL){
        // skip . (leg director curent) si .. (leg director parinte)
        if(strcmp(current_dir->d_name, ".") == 0 || strcmp(current_dir->d_name, "..") == 0){
            continue;
        }
        if (stat(current_dir->d_name, &path_stat) == -1) {
            perror("stat failed");
            exit(-1);
        }
        if(S_ISDIR(path_stat.st_mode) ){
            write_message("\t");
            write_message(current_dir->d_name);
            write_message("\n");
        }
    }
}


int main(int argc, char** argv){
    
    if(argc < 2){
        perror("not enough arguments");
        exit(-1);
    }
    if(strcmp(argv[1], "--add") == 0 && argc == 3){
        add(argv[2]);
        return 0;
    }
    if(strcmp(argv[1], "--list") == 0 && argc == 3){
        list(argv[2]);
        return 0;
    }
    if(strcmp(argv[1], "--view") == 0 && argc == 4){
        view(argv[2], argv[3]);
        return 0;
    }
    if(strcmp(argv[1], "--remove_treasure") == 0 && argc == 4){
        remove_treasure(argv[2], argv[3]);
        return 0;
    }
    if(strcmp(argv[1], "--remove_hunt") == 0 && argc == 3){
        remove_hunt(argv[2]);
        return 0;
    }
    if(strcmp(argv[1], "--list_hunts") == 0 && argc == 2){
        list_hunts();
        return 0;
    }

    return 0;
}