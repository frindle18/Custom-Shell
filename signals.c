#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h> 
#include <unistd.h>

#include "prompt.h" // int handle_error(char* error_message), int get_dir(char dir[])
#include "signals.h"
#include "system.h" // ProcessManager manager

// SIGCHLD handler to manage child process state changes
void sigchld_handler(int sig) {
    int saved_errno = errno;  // Save errno to avoid overwriting it
    pid_t pid;
    int status;

    // Handle any child processes that have changed state (terminated, stopped, etc.)
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        char* command_name = NULL;

        // Loop through the process manager to find the corresponding process
        for (int i = 0; i < manager.count; i++) {
            if (manager.processes[i].pid == pid) {
                command_name = manager.processes[i].command;  // Get the process name

                // If the process has exited or was terminated
                if (WIFEXITED(status) || WIFSIGNALED(status)) {
                    if (WIFEXITED(status)) {
                        // Process exited normally
                        printf(MAGENTA "%s" RESET " with pid " CYAN "%d" RESET " exited normally with status %d\n", command_name, pid, WEXITSTATUS(status));
                    }
                    else if (WIFSIGNALED(status)) {
                        // Process was terminated by a signal
                        printf(BLUE "%s" RESET " with pid " CYAN "%d" RESET " terminated by signal %d\n", command_name, pid, WTERMSIG(status));
                    }

                    manager.processes[i].state = TERMINATED;  // Mark the process as terminated

                    // If the process was in the foreground, reset the foreground PID
                    if (pid == foreground_pid) {
                        foreground_pid = -1;
                    }
                }
                
                // If the process was stopped
                else if (WIFSTOPPED(status)) {
                    manager.processes[i].state = STOPPED;
                    printf(GREEN "%s" RESET " with pid " CYAN "%d" RESET " has been stopped\n", command_name, pid);
                }
                
                // If the process was resumed after being stopped
                else if (WIFCONTINUED(status)) {
                    manager.processes[i].state = RUNNING;
                    printf(YELLOW "%s" RESET " with pid " CYAN "%d" RESET " has been continued\n", command_name, pid);
                }
                
                break; // Exit loop after finding the process
            }
        }
    }

    errno = saved_errno;  // Restore errno to avoid affecting other operations
}

// SIGINT (Ctrl+C) handler to handle interrupt signal for foreground processes
void sigint_handler(int sig) {
    if (foreground_pid > 0) {
        // If there's a foreground process, send it a SIGINT (Ctrl+C)
        if (kill(-foreground_pid, SIGINT) == -1) {
            handle_error("kill() error");
        }
        else {
            printf("\nSent SIGINT to process with pid " CYAN "%d\n" RESET, foreground_pid);
        }
    }
    
    else {
        // If no foreground process, print a message
        printf("\nNo foreground process to interrupt\n");
    }
}

// SIGTSTP (Ctrl+Z) handler to stop (suspend) the foreground process
void sigtstp_handler(int sig) {
    if (foreground_pid > 0) {
        // If there's a foreground process, send it a SIGTSTP (Ctrl+Z)
        if (kill(-foreground_pid, SIGTSTP) == -1) {
            handle_error("kill() error");
        }
        else {
            printf("\nSent SIGTSTP to process with pid " CYAN "%d" RESET ", moving it to background\n", foreground_pid);
            // Mark the process as stopped in the process manager
            for (int i = 0; i < manager.count; i++) {
                if (manager.processes[i].pid == foreground_pid) {
                    manager.processes[i].state = STOPPED;
                    break;
                }
            }
            foreground_pid = -1; // Reset foreground process tracking
        }
    }
    
    else {
        // If no foreground process, print a message
        printf("\nNo foreground process to stop\n");
    }
}

// Sets up signal handling for the shell
// Returns 0 if no errors, else -1
int setup_signal_handling(void) {
    struct sigaction sa;

    // Ignore SIGTTIN and SIGTTOU signals, which are sent when background processes try to read or write to the terminal
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    // Set up the SIGCHLD handler to manage child process state changes
    sa.sa_handler = &sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        fprintf(stderr, "sigaction error for SIGCHLD\n");
        exit(EXIT_FAILURE);
    }

    // Set up the SIGINT (Ctrl+C) handler
    struct sigaction sa_int;
    sa_int.sa_handler = &sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        return handle_error("sigaction() error");
    }

    // Set up the SIGTSTP (Ctrl+Z) handler
    struct sigaction sa_tstp;
    sa_tstp.sa_handler = &sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    if (sigaction(SIGTSTP, &sa_tstp, NULL) == -1) {
        return handle_error("sigaction() error");
    }

    // Ignore the SIGQUIT signal
    signal(SIGQUIT, SIG_IGN);

    // Set the shell's process group ID (PGID) to its own PID
    shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) == -1) {
        return handle_error("setpgid() error");
    }

    // Set the terminal's foreground process group to the shell's PGID
    if (tcsetpgrp(STDIN_FILENO, shell_pgid) == -1) {
        return handle_error("tcsetpgrp() error");
    }

    return 0;
}

// Sends a specified signal to a specified process
// Returns 0 if no errors, else -1
int ping_command(char* pid_str, char* signal_str) {
    // Validate the input for PID and signal number
    if (pid_str == NULL || signal_str == NULL) {
        printf("Usage: ping <pid> <signal_number>\n");
        return -1;
    }

    // Convert the pid_str to pid_t (process ID)
    pid_t pid = (pid_t)atoi(pid_str);
    if (pid <= 0) {
        printf("Invalid PID\n");
        return -1;
    }

    // Convert the signal_str to an integer and limit it to valid signal numbers (mod 32)
    int signal_num = atoi(signal_str);
    signal_num = signal_num % 32;

    // Validate the signal number
    if (signal_num <= 0 || signal_num >= NSIG) {
        printf("Invalid signal number\n");
        return -1;
    }

    // Check if the process exists in the process manager
    int found = 0;
    int process_index = -1;
    for (int i = 0; i < manager.count; i++) {
        if (manager.processes[i].pid == pid) {
            process_index = i;
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("No such process found\n");
        return -1;
    }

    // Send the specified signal to the process
    if (kill(pid, signal_num) == -1) {
        return handle_error("kill() error");
    }

    printf("Sent signal %d to process with pid " CYAN "%d\n", signal_num, pid);

    // Update the state of the process in the manager
    if (process_index != -1) {
        if (signal_num == SIGTSTP) {
            manager.processes[process_index].state = STOPPED;
            printf("Process with pid " CYAN "%d" RESET " has been stopped\n", pid);
        }
        else if (signal_num == SIGCONT) {
            manager.processes[process_index].state = RUNNING;
            printf("Process with pid " CYAN "%d" RESET " has been continued\n", pid);
        }
        else if (signal_num == SIGKILL || signal_num == SIGTERM) {
            manager.processes[process_index].state = TERMINATED;
            printf("Process with pid " CYAN "%d" RESET " has been terminated\n", pid);
        }
    }

    return 0;
}
