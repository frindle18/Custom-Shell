#ifndef INPUT_H
#define INPUT_H

#define NUM_COMMANDS 30
#define MAX_COMMAND_SIZE 2048

typedef struct Command {
    char command[MAX_COMMAND_SIZE];
    char command_type;
} Command;

// Removes leading and trailing spaces from a string
// Modifies the string in-place
void remove_spaces(char* command);

// Splits the input string into commands and their types in case of multiple or background processes
// Additional argument pointer to command_count is passed which will get updated to hold total number of commands
// Returns array of type Command
Command* split(char input[], int* command_count);

#endif // INPUT_H
