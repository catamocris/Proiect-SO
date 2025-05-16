#ifndef TRESURE_HUB_H
#define TRESURE_HUB_H


// -------------------------------------
// hub functions using treasure_manager
// -------------------------------------

void list_hunts_hub();
void list_treasures_hub();
void view_treasure_hub();

// --------------------------------------
// monitor functions
// --------------------------------------

void handle_signal(int sig);
void run_monitor();
void start_monitor();
void stop_monitor();
void exit_command();


#endif