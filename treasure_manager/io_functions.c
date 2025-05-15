#include "io_functions.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>


// writes a message to stdout --------------------------------------------------

void write_message(char *msg){
    write(1, msg, strlen(msg));
}


// reads input from stdin and stores it in the provided buffer -----------------

void read_input(char *input, int size){
    int bytes_read = read(0, input, size-1);
    
    if(bytes_read > 0){
        input[bytes_read] = '\0';                   // adds null terminator
        if (input[bytes_read - 1] == '\n') {        // checks for newline
            input[bytes_read - 1] = '\0';           // replaces newline with null terminator
        }
    }
}
