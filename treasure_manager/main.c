#include "io_functions.h"
#include "treasure_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char** argv){
    if(argc < 2){
        write_message("Usage: \n");
        write_message("\t--add <hunt_id>\n");
        write_message("\t--list <hunt_id>\n");
        write_message("\t--view <hunt_id> <treasure_id>\n");
        write_message("\t--remove_treasure <hunt_id> <treasure_id>\n");
        write_message("\t--remove_hunt <hunt_id>\n");
        write_message("\t--list_hunts\n");
        return 0;
    }
    if(strcmp(argv[1], "--add") == 0 && argc == 3){
        add(argv[2]);
    }
    if(strcmp(argv[1], "--list") == 0 && argc == 3){
        list(argv[2]);
    }
    if(strcmp(argv[1], "--view") == 0 && argc == 4){
        view(argv[2], argv[3]);
    }
    if(strcmp(argv[1], "--remove_treasure") == 0 && argc == 4){
        remove_treasure(argv[2], argv[3]);
    }
    if(strcmp(argv[1], "--remove_hunt") == 0 && argc == 3){
        remove_hunt(argv[2]);
    }
    if(strcmp(argv[1], "--list_hunts") == 0 && argc == 2){
        list_hunts();
    }

    return 0;
}