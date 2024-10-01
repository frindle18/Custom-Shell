#ifndef EXECUTE_H
#define EXECUTE_H

#include "input.h" // struct Command
#include "log.h" // struct Log

#define MAX_BACKGROUND_PROCESSES 30

// Returns 1 if the command is a custom command, else returns 0
int is_custom_command(char* command);

// Calls the function corresponding to the given command
// Returns the time taken to complete the command if it is a shell command executed using execvp
long call_command(char* command, char* args[], Command command_details, char prev_dir[], char home_dir[], Log* command_log, int genius);

// Called by main to execute a command, calls call_command to do so
// Has separate logics for when piping is involved and when it is not
// Saves file descriptors at the beginning of the function and restores them at the end
// Returns the time returned by call_command
long execute_command(Command command_details, char prev_dir[], char home_dir[], Log* command_log);

#endif // EXECUTE_H
