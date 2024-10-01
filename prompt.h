#ifndef PROMPT_H
#define PROMPT_H

#define MAX_NAME_SIZE 256
#define MAX_PATH_SIZE 4096

// Color codes
#define RESET   "\x1b[0m"
#define BLUE    "\x1b[34m"
#define CYAN    "\x1b[36m"
#define GREEN   "\x1b[32m"
#define MAGENTA "\x1b[35m"
#define RED     "\x1b[31m"
#define YELLOW  "\x1b[33m"

// Background color codes
#define BG_GREEN "\x1b[42m"

// Prints error to stderr
// Returns -1 signifying the error
int handle_error(char* error_message);

// Gets username and system name
// Returns 0 if no errors, else -1
int get_system_details(char username[], char system_name[]);

// Gets path of current directory
// Returns 0 if no errors, else -1
int get_dir(char dir[]);

// Displays shell prompt
// Returns 0 if no errors, else -1
int display_prompt(char username[], char system_name[], char home_dir[], char command[], long time);

#endif // PROMPT_H
