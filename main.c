#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "activities.h"
#include "alias.h"
#include "execute.h"
#include "fg_bg.h"
#include "hop.h"
#include "input.h"
#include "log.h"
#include "pipe.h"
#include "proclore.h"
#include "prompt.h"
#include "redirection.h"
#include "reveal.h"
#include "signals.h"
#include "seek.h"
#include "system.h"

int main(void) {
    setup_signal_handling(); // Set up handling for SIGCHILD signal

    // Get username and system name
    char username[MAX_NAME_SIZE];
    char system_name[MAX_NAME_SIZE];

    if (get_system_details(username, system_name) == -1) {
        return -1;
    }

    // Home directory is the directory from which the shell is launched
    char home_dir[MAX_PATH_SIZE];

    if (get_dir(home_dir) == -1) {
        return -1;
    }

    Log* command_log = (Log*)malloc(sizeof(Log));
    command_log->head = command_log->tail = command_log->count = 0; // Initalize struct which logs commands
    load_log_from_file(command_log); // Load commands of previous sessions from file to log

    char prev_dir[MAX_PATH_SIZE]; // Keep track of previous directory
    strcpy(prev_dir, home_dir); // Initialize prev_dir to be home_dir

    Alias* alias_head;
    Function* function_head;

    // Fill in aliases and functions defined in .myshrc
    if (update_aliases_and_functions(&alias_head, &function_head) == -1) {
        return -1;
    }

    display_prompt(username, system_name, home_dir, "", 0); // Initial prompt

    while(1) {
        char input_command[MAX_COMMAND_SIZE];

        if (fgets(input_command, sizeof(input_command), stdin) == NULL) {
            if (feof(stdin)) {
                // Handle EOF (Ctrl+D)
                break; // Exit the shell
            }
        }

        fflush(stdout);

        int log_flag = 0;

        char command_to_log[MAX_COMMAND_SIZE]; // Store the command as it is for logging
        strcpy(command_to_log, input_command);

        remove_spaces(input_command);

        int command_count = 0;
        Command* commands = split(input_command, &command_count);

        long time = 0;

        for (int i = 0; i < command_count; i++) {
            if (strncmp(commands[i].command, "log", strlen("log")) == 0) {
                log_flag = 1; // If any of the commands is "log", don't log that command
            }

            Alias* alias = NULL;
            Function* function = NULL;

            char command_copy[MAX_COMMAND_SIZE];
            strcpy(command_copy, commands[i].command);
            char* shorthand = strtok(command_copy, " ");
            remove_spaces(shorthand); // Get the first word of the command, since in case of functions commands[i].command includes arguments like $1, $2, etc.

            if ((alias = find_alias(commands[i].command, alias_head)) != NULL) {
                time += execute_command(alias->command, prev_dir, home_dir, command_log);
            }

            else if ((function = find_function(shorthand, function_head)) != NULL) {
                update_function(commands[i].command, function);
                for (int j = 0; j < function->num_commands; j++) {
                    time += execute_command(function->command[j], prev_dir, home_dir, command_log);
                }
            }

            else {
                time += execute_command(commands[i], prev_dir, home_dir, command_log);
            }
        }

        display_prompt(username, system_name, home_dir, "sleep", time);

        if (log_flag == 0) {
            add_command_to_log(command_log, command_to_log);
        }
    }

    return 0;
}
