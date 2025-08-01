#include "treasure_hub.h"
#include "../treasure_manager/io_functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(){
    char input[256];

    while(1){
        write_message(">> ");
        read_input(input, sizeof(input));

        if(strcmp(input, "start_monitor") == 0){
            start_monitor();
        } else if(strcmp(input, "list_hunts") == 0){
            list_hunts_hub();
        } else if(strcmp(input, "list_treasures") == 0){
            list_treasures_hub();
        } else if(strcmp(input, "view_treasure") == 0){
            view_treasure_hub();
        } else if(strcmp(input, "calculate_score") == 0){
            calculate_score();
        } else if(strcmp(input, "stop_monitor") == 0){
            stop_monitor();
        } else if(strcmp(input, "exit") == 0){
            exit_command();
        } else{
            write_message("Unknown command\n");
        }
    }
    return 0;
}