#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "execute.h" // long execute_command(Command command_details, char prev_dir[], char home_dir[], Log* command_log)
#include "log.h"
#include "prompt.h" // int handle_error(char* error_message)

// Adds a command to comamnd_log, the struct which is taking care of keeping track of user commands
void add_command_to_log(Log* command_log, char* command) {
    command[strcspn(command, "\n")] = '\0';

    if (command_log->count > 0 && strcmp(command_log->command_history[(command_log->tail - 1 + HISTORY_SIZE) % HISTORY_SIZE], command) == 0) {
        return; // If the user command is same as the previous one, don't add the command to log
    }

    strcpy(command_log->command_history[command_log->tail], command); // Add command to tail

    command_log->tail = (command_log->tail + 1) % HISTORY_SIZE;

    if (command_log->count < HISTORY_SIZE) {
        command_log->count += 1;
    }

    else {
        command_log->head = (command_log->head + 1) % HISTORY_SIZE; // If log is full, move the head
    }

    // Head points at the oldest command
    // Tail points at the position of new command to be added

    save_log_to_file(command_log); // Save log to log.txt every time main calls this function
}

// Resets the head, tail and count of command_log effectively clearing log history
void log_purge(Log* command_log) {
    command_log->head = 0;
    command_log->tail = 0;
    command_log->count = 0;
}

// Executes the specified command in the log by calling execute_command
// Assumption: Commands corresponding to "log execute" do not need to be added to the log
void log_execute(Log* command_log, int index, char prev_dir[], char home_dir[]) {
    int command_index = (command_log->tail - index) % HISTORY_SIZE;

    Command new_command;
    strcpy(new_command.command, command_log->command_history[command_index]);

    execute_command(new_command, prev_dir, home_dir, command_log);
}

// Prints the log starting from head, since it holds the oldest command
void print_log(Log* command_log) {
    int index = command_log->head;

    for (int i = 0; i < command_log->count; i++) {
        printf("%s\n", command_log->command_history[index]);
        index = (index + 1) % HISTORY_SIZE;
    }
}

// Saves the log by writing to file log.txt
int save_log_to_file(Log* command_log) {
    FILE* file = fopen(LOG_FILE_PATH, "w");
    if (file == NULL) {
        return handle_error("fopen() error");
    }

    int index = command_log->head;
    for (int i = 0; i < command_log->count; i++) {
        fprintf(file, "%s\n", command_log->command_history[index]);
        index = (index + 1) % HISTORY_SIZE;
    }

    fclose(file);

    return 0;
}

// Load the log from file into the struct command_log
// Called at the start of the shell session
int load_log_from_file(Log* command_log) {
    FILE *file = fopen(LOG_FILE_PATH, "r");
    if (!file) {
        return handle_error("fopen() error");
    }

    char command[MAX_COMMAND_SIZE];

    while (fgets(command, sizeof(command), file)) {
        add_command_to_log(command_log, command); // Add command to log
    }

    fclose(file);
}

// main calls this function for commands starting with log()
// Calls one of the above functions according to the arguments passed to log
int execute_log(char command[], Log* command_log, char prev_dir[], char home_dir[]) {
    if (strcmp(command, "log") == 0) {
        print_log(command_log);
    }

    else if (strcmp(command, "log purge") == 0) {
        log_purge(command_log);
    }

    
    else if (strncmp(command, "log execute ", strlen("log execute ")) == 0) {
        int index = atoi(command + strlen("log execute "));
        log_execute(command_log, index, prev_dir, home_dir);
    }

    else {
        printf("Invalid command\n");
        return -1;
    }

    return 0;
}
