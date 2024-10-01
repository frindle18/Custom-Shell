#ifndef NEONATE_H
#define NEONATE_H

// Retrieve the most recent process ID (PID) from the /proc directory
// Returns the most recent PID found
int get_recent_pid(void);

// Disable the signals for Ctrl+C (SIGINT) and Ctrl+Z (SIGTSTP)
// Returns -1 in case of errors, else returns 0
int disable_signals(void);

// Set terminal to non-canonical mode to read input character by character
// Returns -1 in case of errors, else returns 0
int set_nonblocking_input(void);

// Restore original terminal settings
// Returns -1 in case of errors, else returns 0
int reset_terminal_settings(void);

// Print the most recent PID at specified intervals until 'x' or Ctrl+D is pressed
// Returns -1 in case of errors, else returns 0
int print_recent_pid(int time_arg);

// Function which handles the neonate command by calling the above implemented functions
// Returns 0 if no errors, else -1
int neonate(char* argv[]);

#endif
