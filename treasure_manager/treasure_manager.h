#ifndef TREASURE_MANAGER_H
#define TREASURE_MANAGER_H

// ---------------------------------------
// treasure struct
// ---------------------------------------

#define ID_LENGTH 15
#define USER_LENGTH 15
#define CLUE_LENGTH 100

typedef struct{
    char id[ID_LENGTH];
    char user[USER_LENGTH];
    float latitude;
    float longitude;
    char clue[CLUE_LENGTH];
    int value;
}Treasure_t;

// ---------------------------------------
// command functions
// ---------------------------------------

void add(char* hunt_id);
void list(char* hunt_id);
void view(char* hunt_id, char* treasure_id);
void remove_treasure(char* hunt_id, char* treasure_id);
void remove_hunt(char* hunt_id);
void list_hunts();

// ---------------------------------------
// helper functions
// ---------------------------------------

void write_treasure_data(Treasure_t treasure);
Treasure_t read_treasure_data(Treasure_t treasure);
void log_action(char* hunt_id, char* log_msg);

#endif