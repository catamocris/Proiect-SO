#include "../treasure_manager/treasure_manager.h"
#include "../treasure_manager/io_functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_USERS 10

typedef struct {
    char user[USER_LENGTH];
    int score;
} User_t;

// -----------------------------------------------------------------------------
// calculates the score of all users in a specific hunt
// -----------------------------------------------------------------------------

int main(int argc, char** argv){

    if(argc != 2) {                                                         // takes Hunt name as argument
        perror("incorrect number of arguments: ./exec <hunt_name>\n");
        exit(-1);
    }

    char path[256];
    sprintf(path, "../treasure_manager/Hunts/%s/treasures.bin", argv[1]);   // path to the treasure file

    int fd = open(path, O_RDONLY);
    if(fd < 0){
        perror("failed to open treasure file\n");
        exit(-1);
    }

    Treasure_t t;
    User_t users[MAX_USERS] = {0};                                          // array to store users
    int count = 0;

    while(read(fd, &t, sizeof(Treasure_t)) == sizeof(Treasure_t)){
        int found = 0;
        for(int i = 0; i < count; i++){                                     // checks if user already exists
            if(strcmp(users[i].user, t.user) == 0){
                users[i].score += t.value;                                  // adds score to existing user
                found = 1;
                break;
            }
        }
        if(!found){                                                         // if user does not exist, adds to array                
            strcpy(users[count].user, t.user);
            users[count].score = t.value;
            count++;
        }
    }

    close(fd);

    for(int i = 0; i < count; i++){
        char msg[256];
        sprintf(msg, "-- User: %s -- Score: %d\n", users[i].user, users[i].score);
        write_message(msg);
    }

    return 0;
}