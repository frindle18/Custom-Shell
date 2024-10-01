#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input.h"

// Removes leading and trailing spaces from a string
// Modifies the string in-place and returns a pointer to it (so that we can use it in functions like strcpy directly)
void remove_spaces(char* command) {
    int i = 0, j = strlen(command);

    while (j > 0 && isspace(command[j - 1])) {
        command[--j] = '\0';
    }

    while (isspace(command[i])) {
        i++;
    }

    if (i > 0) {
        memmove(command, command + i, j - i + 1);
    }
}

// Splits the input string into commands and their types in case of multiple or background processes
// Additional argument pointer to command_count is passed which will get updated to hold total number of commands
// Returns array of type Command
Command* split(char input[], int* command_count) {
    char* command_types = "&;"; // Tokenize based on background (&) or sequential (;) processes

    char input_copy[MAX_COMMAND_SIZE];
    strcpy(input_copy, input); // strtok modifies string in place

    Command* commands = (Command*)malloc(NUM_COMMANDS * sizeof(Command));

    char* token = strtok(input, command_types);

    while (token != NULL) {
        strcpy(commands[*command_count].command, token);
        remove_spaces(commands[*command_count].command);

        commands[*command_count].command_type = input_copy[token - input + strlen(token)]; // Get the type of the process, token points to the start of that specific process, - input to index into input_copy

        (*command_count)++;

        token = strtok(NULL, command_types);
    }

    return commands;
}
