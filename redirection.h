#ifndef REDIRECTION_H
#define REDIRECTION_H

// Parses a command involving input or output redirection
// Gets the args to be passed to execvp, the input file to read from, the output file to write to, and whether to append or overwrite the file being written to
// Returns 0 if no errors, else -1
int parse_command(char* command, char* args[], char** input_file, char** output_file, int* append);

// Handles redirection of input or output
// Returns -1 in case of any error, else returns 0
int handle_redirection(char* input_file, char* output_file, int append);

// Restore file descriptors to the ones in saved_stdin and saved_stdout
void reset_redirection(int saved_stdin, int saved_stdout);

#endif // REDIRECTION_H
