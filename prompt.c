#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "prompt.h"

// Prints error to stderr
// Returns -1 signifying the error
int handle_error(char* error_message) {
    fprintf(stderr, "%s: %s\n", error_message, strerror(errno));
    return -1;
}

// Gets username and system name
// Returns 0 if no errors, else -1
int get_system_details(char username[], char system_name[]) {
    struct passwd* p; // Stores information about user account
    uid_t uid = getuid(); // Get user id of the calling process

    if ((p = getpwuid(uid)) == NULL) {
        return handle_error("getpwuid() error"); // Let the calling function know about the error
    }

    else {
        strcpy(username, p->pw_name);
    }

    int hostname_ret = gethostname(system_name, MAX_NAME_SIZE);

    if (hostname_ret == -1) {
        return handle_error("gethostname() error");
    }

    return 0;
}

// Gets path of current directory
// Returns 0 if no errors, else -1
int get_dir(char dir[]) {
    if (getcwd(dir, MAX_PATH_SIZE) == NULL) {
        return handle_error("getcwd() error");
    }

    return 0;
}

// Displays shell prompt
// Returns 0 if no errors, else -1
int display_prompt(char username[], char system_name[], char home_dir[], char command[], long time) {
    printf("<" GREEN "%s@%s" RESET ":", username, system_name);

    // Current directory
    char current_dir[MAX_PATH_SIZE];
    if (get_dir(current_dir) == -1) {
        return -1;
    }

    // Print prompt
    if (strcmp(current_dir, home_dir) == 0) {
        printf(BLUE "~" RESET);
    }

    else if (strncmp(current_dir, home_dir, strlen(home_dir)) == 0) {
        // Compare only as much as home dir's length to check whether current_dir is a subdirectory of home_dir
        printf(BLUE "~%s" RESET, current_dir + strlen(home_dir)); // Pointer points to the portion of current_dir that comes after home_dir
    }

    else {
        // Current directory is outside the home directory
        printf(BLUE "%s" RESET, current_dir);
    }

    if (time > 2) { // Display the previous command and the time taken by it in the prompt if it was greater than 2s
        printf(RED " %s : %lds" RESET, command, time);
    }

    printf("> ");

    return 0;
}
