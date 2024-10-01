#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "prompt.h" // int handle_error(char* error_message)

// Parses a command involving input or output redirection
// Gets the args to be passed to execvp, the input file to read from, the output file to write to, and whether to append or overwrite the file being written to
// Returns 0 if no errors, else -1
int parse_command(char* command, char* args[], char** input_file, char** output_file, int* append) {
    *append = 0;
    int i = 0;

    char* token = strtok(command, " ");
    while (token != NULL) {
        if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                *output_file = strdup(token);
            }
            else {
                fprintf(stderr, "Error: No output file specified after '>'\n");
                return -1;
            }
        }

        else if (strcmp(token, ">>") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                *output_file = strdup(token);
                *append = 1;
            }
            else {
                fprintf(stderr, "Error: No output file specified after '>>'\n");
                return -1;
            }
        }

        else if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            if (token) {
                *input_file = strdup(token);
            }
            else {
                fprintf(stderr, "Error: No input file specified after '<'\n");
                return -1;
            }
        }

        else {
            args[i++] = strdup(token);
        }

        token = strtok(NULL, " ");
    }

    args[i] = NULL; // Null-terminate args for execvp

    return 0;
}

// Handles redirection of input or output
// Returns -1 in case of any error, else returns 0
int handle_redirection(char* input_file, char* output_file, int append) {
    // If there is an input file to redirect to, open the file
    if (input_file) {
        int fd_in = open(input_file, O_RDONLY);

        if (fd_in == -1) {
            return handle_error("open() error");
        }

        // Redirect stdin (STDIN_FILENO) to the input file descriptor (fd_in)
        if (dup2(fd_in, STDIN_FILENO) == -1) {
            close(fd_in);
            return handle_error("dup2() error");
        }

        close(fd_in); // Close the input file descriptor since it's duplicated now
    }

    // If there is an output redirection file provided
    if (output_file) {
        // O_APPEND if append is true, otherwise O_TRUNC (truncate file)
        int fd_out = open(output_file, O_CREAT | O_WRONLY | (append ? O_APPEND : O_TRUNC), 0644);
 
        if (fd_out == -1) {
            return handle_error("open() error");
        }

        // Redirect stdout (STDOUT_FILENO) to the output file descriptor (fd_out)
        if (dup2(fd_out, STDOUT_FILENO) == -1) {
            close(fd_out);
            return handle_error("dup2() error");
        }

        close(fd_out);
    }

    return 0;
}

// Restore file descriptors to the ones in saved_stdin and saved_stdout
void reset_redirection(int saved_stdin, int saved_stdout) {
    if (saved_stdin != -1) {
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
    }
    
    if (saved_stdout != -1) {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
}
