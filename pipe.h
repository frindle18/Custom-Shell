#ifndef PIPE_H
#define PIPE_H

#include "log.h"

#define MAX_ARGS 128
#define MAX_PIPES 16

// Splits a command involving pipes into individual commands
void split_commands(char* command, char* commands[], int* num_commands);

// Sets up the pipe for the ith command
// Returns 0 if no errors, otherwise returns -1
int setup_pipe(int i, int num_commands, int saved_stdin, int saved_stdout, int pipefd[]);

// Piping
// Sets up and executes a series of piped commands by splitting the input, creating pipes between commands and handling I/O redirection
// Returns 0 if no errors, else -1
int execute_pipeline(char* command_line, Command command_details, char prev_dir[], char home_dir[], Log* command_log);

#endif // PIPE_H
