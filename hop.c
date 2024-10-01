#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "hop.h"
#include "prompt.h" // int handle_error(char* error_message), int get_dir(char dir[])

// Prints the current working directory
// Returns 0 if no errors, else -1
int print_cwd(void) {
    char cwd[MAX_PATH_SIZE];
    
    if (get_dir(cwd) == -1) {
        return -1;
    }
    printf("%s\n", cwd);

    return 0;
}

// Replaces the "~" of a user-inputed path with the path of the home directory
// input_dir is passed as an argument to the function and gets filled after execution
void handle_home_paths(char input_dir[], char home_dir[]) {
    char path_from_home[MAX_PATH_SIZE];

    strcpy(path_from_home, input_dir + 1);
    strcpy(input_dir, home_dir);
    strcat(input_dir, path_from_home);
} // Also takes care of "hop ~", then new_dir = home_dir

// Implements a single change of directory using chdir()
// Assumption: Also has to handle inputs like "hop ~ test"
// Returns -1 in case of any error, else returns 0
int change_dir(char dir[], char prev_dir[], char home_dir[]) {
    char new_dir[MAX_PATH_SIZE];
    strcpy(new_dir, dir);

    // Paths starting with ~
    if (dir[0] == '~') {
        handle_home_paths(new_dir, home_dir);
    }

    // new_dir now has the proper full path to the required directory

    if (get_dir(prev_dir) == -1) {
        return -1;
    } // prev_dir = current directory, since we're about to change directories

    if (chdir(new_dir) == 0) {
        if (print_cwd() == -1) {
            return -1;
        }
    }

    else {
        return handle_error("chdir() error");
    }

    return 0;
}

// Scans the hop command and makes calls to change_dir after tokenizing the input (for sequential hop)
// Assumption: Paths/names do not contain any whitespace characters, otherwise we'll run into errors with strtok
// Returns -1 in case of any error, else returns 0
int hop(char command[], char prev_dir[], char home_dir[]) {
    char dir[MAX_PATH_SIZE]; // Directory to change into

    if (strcmp(command, "hop") == 0) {
        strcpy(dir, "~");
    } // If no argument is present, hop into home

    // Change into previous directory
    else if (strcmp(command, "hop -") == 0) {
        strcpy(dir, prev_dir);
    }

    else if (strncmp(command, "hop ", strlen("hop ")) == 0) {
        strncpy(dir, command + strlen("hop "), strlen(command) - strlen("hop ")); // Extract part of the string after "hop " which is the directory to change into
        dir[strlen(command) - strlen("hop ")] = '\0'; 
    }

    else {
        printf("Invalid command\n");
        return -1;
    }

    char* token = strtok(dir, " ");

    while (token != NULL) {
        if (change_dir(token, prev_dir, home_dir) == -1) {
            return -1;
        }
        token = strtok(NULL, " ");
    } // Sequentially change directories from left to right
}
