#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>

#include "prompt.h" // int handle_error(char* error_message);
#include "redirection.h"
#include "system.h" // void reset_redirection(int saved_stdin, int saved_stdout)

ProcessManager manager = { .count = 0 };

// Global variables to track the foreground process and shell process group ID
pid_t foreground_pid = -1;
pid_t shell_pgid;

// Executes a shell command, and also calculates the time taken to do so
// The parameter genius informs the function if the command is the last individual command in a piped command
long execute_shell_command(char command[], char command_type, char* args[], int genius) {
    int background = 0;

    // Determine if the command is meant to be executed in the background
    if (command_type == '&') {
        background = 1;
    }

    int rc = fork();  // Fork the process

    if (rc < 0) {
        return handle_error("fork() error");  // Error during fork
    }

    else if (rc == 0) {
        // Child process
        if (setpgid(0, 0) == -1) {
            return handle_error("setpgid() error");  // Set process group ID for the child
        }

        // Reset signal handlers for the child process to default
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        // Execute the given command
        execvp(args[0], args);

        // If execvp fails
        printf("ERROR: not a valid command\n");
        exit(1);
    }

    else {
        // Parent process

        // Store process details if variable genius is set, since it represents the last individual command in a piped command
        if (genius == 1) {
            if (manager.count >= MAX_PROCESSES) {
                fprintf(stderr, "Maximum number of processes reached.\n");
                return -1;
            }
            
            // Store the process ID, command, and state in the process manager
            manager.processes[manager.count].pid = rc;
            manager.processes[manager.count].command = strdup(command);
            manager.processes[manager.count].state = RUNNING;
            manager.count++;
        }

        // Handle background processes
        if (background == 1) {
            int saved_stdin = dup(STDIN_FILENO);
            int saved_stdout = dup(STDOUT_FILENO);
            int terminal_fd = open("/dev/tty", O_RDWR);

            // Redirect terminal I/O for the background process
            if (terminal_fd != -1) {
                dup2(terminal_fd, STDIN_FILENO);
                dup2(terminal_fd, STDOUT_FILENO);
                close(terminal_fd);
            }

            // Print the PID of the background process if it is either a non-piped command or the last individual command in a piped command
            if (genius == 1) {
                printf(CYAN "[%d]" RESET "\n", rc);
            }

            // Restore original stdin and stdout
            reset_redirection(saved_stdin, saved_stdout);

            return -1;
        }
        
        else {
            // Foreground process
            foreground_pid = rc;

            // Attempt to set the child process as the foreground process
            if (genius != -1 && genius != 1) { // If it's not a piped process
                if (tcsetpgrp(STDIN_FILENO, rc) == -1) {
                    return handle_error("tcsetpgrp() error");
                }
            }

            struct timeval start, end;
            gettimeofday(&start, NULL);  // Record the start time of command execution

            int status;
            // Wait for the child process to finish
            if (waitpid(rc, &status, WUNTRACED | WCONTINUED) == -1) {
                perror("waitpid");
            }

            gettimeofday(&end, NULL);  // Record the end time
            long seconds = end.tv_sec - start.tv_sec;  // Calculate the execution time

            // If the process was stopped, move it to the background
            if (WIFSTOPPED(status)) {
                for (int i = 0; i < manager.count; i++) {
                    if (manager.processes[i].pid == rc) {
                        manager.processes[i].state = STOPPED;
                        printf("Process with pid " CYAN "%d" RESET " has been stopped and moved to background.\n", rc);
                        break;
                    }
                }
            }

            // If the process terminated, update its state
            else {
                for (int i = 0; i < manager.count; i++) {
                    if (manager.processes[i].pid == rc) {
                        manager.processes[i].state = TERMINATED;
                        break;
                    }
                }
            }

            foreground_pid = -1;

            // If the shell is still in control of the terminal, set the shell back as the foreground process group
            if (tcgetpgrp(STDIN_FILENO) == getpid()) {
                if (tcsetpgrp(STDIN_FILENO, shell_pgid) == -1) {
                    return handle_error("tcsetpgrp() error");
                }
            }

            return seconds;  // Return the time the command took to execute
        }
    }

    return 0;
}
