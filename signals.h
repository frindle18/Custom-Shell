#ifndef SIGNALS_H
#define SIGNALS_H

// SIGCHLD handler to manage child process state changes
void sigchld_handler(int sig);

// SIGINT (Ctrl+C) handler to handle interrupt signal for foreground processes
void sigint_handler(int sig);

// SIGTSTP (Ctrl+Z) handler to stop (suspend) the foreground process
void sigtstp_handler(int sig);

// Sets up signal handling for the shell
// Returns 0 if no errors, else -1
int setup_signal_handling(void);

// Sends a specified signal to a specified process
// Returns 0 if no errors, else -1
int ping_command(char* pid_str, char* signal_str);

#endif // SIGNALS_H
