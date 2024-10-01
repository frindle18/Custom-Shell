#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "execute.h" // long call_command(char* command, char* args[], Command command_details, char prev_dir[], char home_dir[], Log* command_log)
#include "input.h" // #define MAX_COMMAND_SIZE 2048
#include "log.h" // struct Log
#include "pipe.h"
#include "prompt.h" // int handle_error(char* error_message)
#include "redirection.h" // All functions there

// Splits a command involving pipes into individual commands
void split_commands(char* command, char* commands[], int* num_commands) {
    char* token = strtok(command, "|");
    
    while (token != NULL && *num_commands < MAX_PIPES) {
        remove_spaces(token);
        commands[(*num_commands)++] = token;
        token = strtok(NULL, "|");
    }
}

// Sets up the pipe for the ith command
// Returns 0 if no errors, otherwise returns -1
int setup_pipe(int i, int num_commands, int saved_stdin, int saved_stdout, int pipefd[]) {
    // If the command is not the last command, the output of this command should be piped to the next command
    // So a new pipe is created using the pipe system call
    if (i < num_commands - 1) {
        if (pipe(pipefd) == -1) {
            return handle_error("pipe() error");
        }
    }

    // If this is the last command, there's no need for a new pipe
    // Instead, duplicate saved_stdin and saved_stdout for pipefd
    // This is done so that the last command behaves as if it were running in a normal non-pipeline environment
    else {
        pipefd[0] = dup(saved_stdin);
        pipefd[1] = dup(saved_stdout);
    }

    return 0;
}

// Piping
// Sets up and executes a series of piped commands by splitting the input, creating pipes between commands and handling I/O redirection
// Returns 0 if no errors, else -1
int execute_pipeline(char* command_line, Command command_details, char prev_dir[], char home_dir[], Log* command_log) {
    char* commands[MAX_PIPES]; // Array of individual commands (split at every pipe)
    int num_commands = 0;

    // Split the command line into individual commands (stored in array commands)
    split_commands(command_line, commands, &num_commands);

    int saved_stdin = dup(STDIN_FILENO);
    int saved_stdout = dup(STDOUT_FILENO);

    int input_fd = saved_stdin; // Start with original stdin

    for (int i = 0; i < num_commands; i++) {
        char* args[MAX_ARGS]; // args for execvp
        char* input_file = NULL; // Input file to read input from (if there)
        char* output_file = NULL; // Output file to redirect output to (if there)
        int append = 0; // Append mode for output file

        char command_copy[MAX_COMMAND_SIZE];
        strcpy(command_copy, commands[i]); // parse_command changes string in place, so a copy of the command is passed

        // Parse the command for arguments and redirection
        parse_command(command_copy, args, &input_file, &output_file, &append);

        int pipefd[2];
        if (setup_pipe(i, num_commands, saved_stdin, saved_stdout, pipefd) == -1) {
            reset_redirection(saved_stdin, saved_stdout); // Restore original stdin and stdout in case of error
            return -1;
        }

        // Save current stdin and stdout before redirection
        int temp_stdin = dup(STDIN_FILENO);
        int temp_stdout = dup(STDOUT_FILENO);

        // If not the first command, set input_fd to the read end of the previous pipe
        if (i > 0) {
            dup2(input_fd, STDIN_FILENO); // input_fd contains pipefd[0] of previous pipe
        }

        // If not the last command, set stdout to the write end of the current 
        // Close write end of the pipe in the current process
        if (i < num_commands - 1) {
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
        }

        // Handle input/output redirection specified in the command
        if (handle_redirection(input_file, output_file, append) == -1) {
            reset_redirection(saved_stdin, saved_stdout);
            close(temp_stdin);
            close(temp_stdout);
            return -1;
        }

        // Execute the command
        // If the command is the last of all pipes, convey that to call_command
        // call_command passes on that information to execute_shell_command, which adds this last individual command in the entire pipe command to the background process list if it is one
        if (i == num_commands - 1) {
            call_command(args[0], args, command_details, prev_dir, home_dir, command_log, 1);
        }

        else {
            call_command(args[0], args, command_details, prev_dir, home_dir, command_log, -1);
        }

        // Restore stdin and stdout to their original values
        reset_redirection(temp_stdin, temp_stdout);

        // If input_fd is different from the original stdin (saved_stdin), it means input_fd is the read end of a pipe from a previous command.
        if (input_fd != saved_stdin) {
            close(input_fd);
        }

        input_fd = pipefd[0]; // The read end of the pipe becomes the input for the next command
    }

    // Restore original stdin and stdout
    reset_redirection(saved_stdin, saved_stdout);

    return 0;
}
