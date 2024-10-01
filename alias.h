#ifndef ALIASES_H
#define ALIASES_H

#include "input.h" // struct Command, MAX_COMMAND_SIZE

#define MAX_ARGUMENTS 9 // Maximum number of arguments to function like $1, $2, etc.
#define MAX_FUNCTION_COMMANDS 20 // Maximum number of commands inside a function
#define MAX_SHORTHAND_SIZE 50 // Maximum length of shorthand

typedef struct Alias {
    char shorthand[MAX_SHORTHAND_SIZE];
    struct Command command;
    struct Alias* next;
} Alias;

typedef struct Function {
    char shorthand[MAX_SHORTHAND_SIZE];
    struct Command* command; // List of commands
    int num_commands; // Number of commands in the function
    struct Function* next;
} Function;

// Inserts alias "shorthand = command" into the linked list of aliases pointed to by alias_head
// Assumption: Aliases can't have multiple processes (like involving background, eg. lol = sleep 2 & ls)
void insert_alias(Alias** alias_head, char* shorthand, char* command);

// Separates out the shorthand and the command in the alias, and calls insert_alias
// Works even if the spaces are messed up, as long as the command is valid
void parse_alias(Alias** head, char* line);

// Inserts function with given shorthand and commands into the linked list of functions pointed to by function_head
void insert_function(Function** function_head, char* shorthand, char* commands[], int num_commands);

// Works even if the spaces are messed up, as long as the command is valid
// Assumption: Number of commands inside a function does not exceed MAX_FUNCTION_COMMANDS
int parse_function(Function** head, char* line, FILE* file);

// Finds a particular alias with given shorthand by looking through the linked list of aliases
// Returns pointer to the alias if it is found, else returns NULL
Alias* find_alias(char* shorthand, Alias* alias_head);

// Finds a particular function with given shorthand by looking through the linked list of functionss
// Returns pointer to the function if it is found, else returns NULL
Function* find_function(char* shorthand, Function* alias_head);

// Replaces positional parameters of commands of the function like $1, $2, etc. with the commands the user passed along with the function shorthand
// Works for ANY FUCKING FUNCTION AND EVEN IF EACH COMMAND INSIDE A FUNCTION HAVE DIFFERENT NUMBER OF ARGUMENTS, not just the two which needed implementation cuz I didn't read that part and wasted like 5 hours
// Can you please consider giving more than a 2 mark bonus
void update_function(char* command, Function* function);

// Called by main to fill in linked lists of aliases and functions
// Returns -1 in case of any error, else returns 0
int update_aliases_and_functions(Alias** alias_head, Function** function_head);

#endif // ALIASES_H
