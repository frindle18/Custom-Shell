#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "activities.h"
#include "execute.h"
#include "fg_bg.h"
#include "hop.h"
#include "iman.h"
#include "input.h"
#include "log.h"
#include "neonate.h"
#include "pipe.h"
#include "proclore.h"
#include "prompt.h"
#include "redirection.h"
#include "reveal.h"
#include "seek.h"
#include "signals.h"
#include "system.h"

// Returns 1 if the command is a custom command, else returns 0
int is_custom_command(char* command) {
    if (strcmp(command, "activities") == 0 || strcmp(command, "bg") == 0 || strcmp(command, "fg") == 0 || strcmp(command, "hop") == 0 || strcmp(command, "iMan") == 0 || strcmp(command, "log") == 0 || strcmp(command, "neonate") == 0 || strcmp(command, "ping") == 0 || strcmp(command, "proclore") == 0 || strcmp(command, "reveal") == 0 || strcmp(command, "seek") == 0) {
        return 1;
    }
    return 0;
}

// Calls the function corresponding to the given command
// Returns the time taken to complete the command if it is a shell command executed using execvp
long call_command(char* command, char* args[], Command command_details, char prev_dir[], char home_dir[], Log* command_log, int genius) {
    long seconds = 0;

    if (strncmp(command, "activities", strlen("activities")) == 0) {
        activities();
    }

    else if (strncmp(command, "bg", strlen("bg")) == 0) {
        background_command(args[1]);
    }

    else if (strncmp(command, "fg", strlen("fg")) == 0) {
        foreground_command(args[1]);
    }

    else if (strncmp(command, "hop", strlen("hop")) == 0) {
        hop(command, prev_dir, home_dir);
    }

    else if (strncmp(command, "iMan", strlen("iMan")) == 0) {
        iMan(args[1]);
    }

    else if (strncmp(command, "log", strlen("log")) == 0) {
        execute_log(command, command_log, prev_dir, home_dir);
    }
    
    else if (strncmp(command, "neonate", strlen("neonate")) == 0) {
        neonate(args);
    }

    else if (strncmp(command, "ping", strlen("ping")) == 0) {
        ping_command(args[1], args[2]);
    }

    else if (strncmp(command, "proclore", strlen("proclore")) == 0) {
        proclore(command);
    }

    else if (strncmp(command, "reveal", strlen("reveal")) == 0) {
        reveal(command, prev_dir, home_dir);
    }
    
    else if (strncmp(command, "seek", strlen("seek")) == 0) {
        seek(command, home_dir);
    }

    else {
        seconds = execute_shell_command(command, command_details.command_type, args, genius);
    }

    return seconds;
}

// Called by main to execute a command, calls call_command to do so
// Has separate logics for when piping is involved and when it is not
// Saves file descriptors at the beginning of the function and restores them at the end
// Returns the time returned by call_command
long execute_command(Command command_details, char prev_dir[], char home_dir[], Log* command_log) {
    char command[MAX_COMMAND_SIZE];
    remove_spaces(command_details.command);
    strcpy(command, command_details.command);

    int saved_stdin = dup(STDIN_FILENO);
    int saved_stdout = dup(STDOUT_FILENO);

    // Check if the command contains a pipe
    if (strchr(command, '|') != NULL) {
        execute_pipeline(command, command_details, prev_dir, home_dir, command_log);
        reset_redirection(saved_stdin, saved_stdout);
        return 0;
    }

    // If no pipes, just handle I/O indirection and execute the command
    // This is there specifically for hop, since if we integrate the logic for hop and piping, the hop happens for the child process and not the parent process

    char* args[MAX_BACKGROUND_PROCESSES]; // For execvp arguments
    char* input_file = NULL;
    char* output_file = NULL;
    int append = 0;

    char command_copy[MAX_COMMAND_SIZE];
    strcpy(command_copy, command); // parse_command modifies command_copy in place

    // Parse the command for arguments and redirection
    parse_command(command_copy, args, &input_file, &output_file, &append);

    if (handle_redirection(input_file, output_file, append) == -1) {
        reset_redirection(saved_stdin, saved_stdout); // Restore file descriptors in case of error
        return -1;
    }

    command[strcspn(command, "<")] = '\0'; 
    command[strcspn(command, ">")] = '\0'; // Pass in the correct command to call_command after getting rid of < and >, since we've dealt with those redirections
    
    long seconds = call_command(command, args, command_details, prev_dir, home_dir, command_log, 1);

    // Reset redirection after the command has executed
    reset_redirection(saved_stdin, saved_stdout);

    return seconds;
}
