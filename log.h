#ifndef LOG_H
#define LOG_H

#include "input.h" // MAX_COMMAND_SIZE

#define HISTORY_SIZE 15
#define LOG_FILE_PATH "/mnt/c/Users/sudar/OneDrive - International Institute of Information Technology/College/2-1/Projects/Custom-Shell/log.txt"

typedef struct Log {
    char command_history[HISTORY_SIZE][MAX_COMMAND_SIZE];
    int head;
    int tail;
    int count;
} Log;

// Adds a command to comamnd_log, the struct which is taking care of keeping track of user commands
void add_command_to_log(Log* command_log, char* command);

// Resets the head, tail and count of command_log effectively clearing log history
void log_purge(Log* command_log);

// Executes the specified command in the log by calling execute_command
// Assumption: Commands corresponding to "log execute" do not need to be added to the log
void log_execute(Log* command_log, int index, char prev_dir[], char home_dir[]);

// Prints the log starting from head, since it holds the oldest command
void print_log(Log* command_log);

// Saves the log by writing to file log.txt
int save_log_to_file(Log* command_log);

// Load the log from file into the struct command_log
// Called at the start of the shell session
int load_log_from_file(Log* command_log);

// main calls this function for commands starting with log()
// Calls one of the above functions according to the arguments passed to log
int execute_log(char command[], Log* command_log, char prev_dir[], char home_dir[]);

#endif // LOG_H
