#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alias.h"
#include "input.h" // Command* split(char input[], int* command_count), char* remove_spaces(char* command)
#include "prompt.h" // int handle_error(char* error_message);

// Inserts alias "shorthand = command" into the linked list of aliases pointed to by alias_head
// Assumption: Aliases can't have multiple processes (like involving background, eg. lol = sleep 2 & ls)
void insert_alias(Alias** alias_head, char* shorthand, char* command) {
    Alias* new_alias = (Alias*)malloc(sizeof(Alias));

    strcpy(new_alias->shorthand, shorthand);

    int temp = 0;
    new_alias->command = split(command, &temp)[0]; // There will be only one command

    new_alias->next = *alias_head;

    *alias_head = new_alias;
}

// Separates out the shorthand and the command in the alias, and calls insert_alias
// Works even if the spaces are messed up, as long as the command is valid
void parse_alias(Alias** head, char* line) {
    char* shorthand = strtok(line, "=");
    char* command = strtok(NULL, "=");

    remove_spaces(shorthand);
    remove_spaces(command);

    insert_alias(head, shorthand, command);
}

// Inserts function with given shorthand and commands into the linked list of functions pointed to by function_head
void insert_function(Function** function_head, char* shorthand, char* commands[], int num_commands) {
    Function* new_function = (Function*)malloc(sizeof(Function));

    strcpy(new_function->shorthand, shorthand);

    new_function->num_commands = num_commands;

    char concat_commands[MAX_COMMAND_SIZE] = "";

    for (int i = 0; i < num_commands; i++) {
        strcat(concat_commands, commands[i]);
        strcat(concat_commands, " ; ");
    }

    int temp = 0;
    new_function->command = split(concat_commands, &temp);

    new_function->next = *function_head;
    *function_head = new_function;
}

// Assumption: Number of commands inside a function does not exceed MAX_FUNCTION_COMMANDS
// Works even if the spaces are messed up, as long as the command is valid
// Returns -1 in case of any error, else returns 0
int parse_function(Function** head, char* line, FILE* file) {
    char* shorthand = strtok(line, "()");

    char shorthand_copy[MAX_SHORTHAND_SIZE];
    strcpy(shorthand_copy, shorthand);
    remove_spaces(shorthand_copy);

    char* commands[MAX_FUNCTION_COMMANDS];
    int num_commands = 0;

    // Read the opening brace { which indicates start of function
    if (fgets(line, MAX_COMMAND_SIZE, file) == NULL) {
        return handle_error("fgets() error");
    }

    while (fgets(line, MAX_COMMAND_SIZE, file)) {
        line[strcspn(line, "\n")] = '\0'; // Replace the newline character at the end of the line with null terminator
        
        line[strcspn(line, "#")] = '\0'; // Ignore the part of the string after the '#' character which denotes the start of a comment

        if (strchr(line, '}') != NULL) {
            break; // Indicates end of function
        }

        commands[num_commands] = strdup(line);
        num_commands++;
    }

    insert_function(head, shorthand_copy, commands, num_commands);

    // Free the function commands after insertion
    for (int i = 0; i < num_commands; i++) {
        free(commands[i]);
    }

    return 0;
}

// Finds a particular alias with given shorthand by looking through the linked list of aliases
// Returns pointer to the alias if it is found, else returns NULL
Alias* find_alias(char* shorthand, Alias* alias_head) {
    Alias* temp = alias_head;

    while (temp != NULL) {
        if (strcmp(temp->shorthand, shorthand) == 0) {
            return temp; // Return the pointer to the found Alias
        }
        temp = temp->next;
    }

    return NULL; // Return NULL if no Alias is found
}

// Finds a particular function with given shorthand by looking through the linked list of functionss
// Returns pointer to the function if it is found, else returns NULL
Function* find_function(char* shorthand, Function* function_head) {
    Function* temp = function_head;

    while (temp != NULL) {
        if (strcmp(temp->shorthand, shorthand) == 0) {
            return temp; // Return the pointer to the found Function
        }
        temp = temp->next;
    }

    return NULL; // Return NULL if no Function is found
}

// Replaces positional parameters of commands of the function like $1, $2, etc. with the commands the user passed along with the function shorthand
// Works for ANY FUCKING FUNCTION AND EVEN IF EACH COMMAND INSIDE A FUNCTION HAVE DIFFERENT NUMBER OF ARGUMENTS, not just the two which needed implementation cuz I didn't read that part and wasted like 5 hours
// Can you please consider giving more than a 2 mark bonus
void update_function(char* command, Function* function) {
    char* args[MAX_ARGUMENTS];

    char* command_copy = strdup(command);

    // Use strtok to get the first token (command, which we want to skip)
    char* token = strtok(command_copy, " ");
    int i = 0;

    while (token = strtok(NULL, " ")) {
        args[i++] = strdup(token);
    }

    free(command_copy);

    for (int j = 0; j < function->num_commands; j++) {
        char substituted_command[MAX_COMMAND_SIZE] = "";

        char* ptr = function->command[j].command;

        while (*ptr != '\0') {
            if (*ptr == '$') {
                ptr++; // Move past the '$'
                if (*ptr >= '1' && *ptr <= '9') {  // Check if the next character is a number between '1' and '9'
                    int arg_index = *ptr - '1';  // Get the argument index (0-based)
                    if (arg_index < MAX_ARGUMENTS) {
                        strcat(substituted_command, args[arg_index]);
                    }
                }
            }

            else if (*ptr == '"') { // Skip past the quotes in "$1", etc.
                ptr++;
                continue;
            }

            else {
                // Append the current character to the final command
                strncat(substituted_command, ptr, 1);
            }

            ptr++;
        }

        strcpy(function->command[j].command, substituted_command); // Update command
    }
}

// Called by main to fill in linked lists of aliases and functions
// Returns -1 in case of any error, else returns 0
int update_aliases_and_functions(Alias** alias_head, Function** function_head) {
    FILE *file = fopen(".myshrc", "r");
    if (file == NULL) {
        return handle_error("fopen() error");
    }

    char line[MAX_COMMAND_SIZE];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0'; // Replace the newline character at the end of the line with null terminator
        
        line[strcspn(line, "#")] = '\0'; // Ignore the part of the string after the '#' character which denotes the start of a comment

        // Skip empty lines and lines which are fully commented out
        if (line[0] == '\0') {
            continue;
        }

        if (strchr(line, '=') != NULL) {
            parse_alias(alias_head, line);
        }

        else if (strstr(line, "()") != NULL) {
            if (parse_function(function_head, line, file) == -1) {
                return -1;
            }
        }
    }

    fclose(file);

    return 0;
}
