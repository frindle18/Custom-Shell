#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "fg_bg.h"
#include "prompt.h" // int handle_error(char* error_message)
#include "system.h" // struct Process

// Brings the running or stopped background process with corresponding pid to foreground
int foreground_command(char* pid_str) {
    if (pid_str == NULL) {
        // If no PID is provided, print usage instructions
        printf("Usage: fg <pid>\n");
        return -1;
    }

    pid_t pid = atoi(pid_str);

    if (pid <= 0) {
        printf("Invalid PID\n");
        return -1;
    }

    // Check if the process exists by sending signal 0 to it
    if (kill(pid, 0) == -1) {
        printf("No such process found\n");
        return -1;
    }

    // Get process group ID
    pid_t pgid = getpgid(pid);

    if (pgid == -1) {
        return handle_error("getpgid() error");
    }

    // Bring the process group to the foreground by assigning terminal control to it
    if (tcsetpgrp(STDIN_FILENO, pgid) == -1) {
        return handle_error("tcsetpgrp() error"); // Error bringing process to foreground
    }

    // Send a SIGCONT signal to the process group to resume execution if it was stopped
    if (kill(-pgid, SIGCONT) == -1) {
        return handle_error("kill() error");
    }

    // Set the global variable `foreground_pid` to track the current foreground process
    foreground_pid = pid;

    // Wait for the process to finish or be stopped (WUNTRACED flag catches stopped processes), catching signals like termination or suspension
    int status;
    if (waitpid(pid, &status, WUNTRACED) == -1) {
        return handle_error("waitpid() error");
    }

    // After the process is done, return terminal control back to the shell's process group
    if (tcsetpgrp(STDIN_FILENO, shell_pgid) == -1) {
        return handle_error("tcsetpgrp() error");
    }

    // Update the process manager's state of the process based on the exit status (terminated or stopped)
    for (int i = 0; i < manager.count; i++) {
        if (manager.processes[i].pid == pid) {
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                // If the process exited or was killed by a signal, mark it as terminated
                manager.processes[i].state = TERMINATED;
            }
            else if (WIFSTOPPED(status)) {
                // If the process was stopped (e.g., via Ctrl+Z), mark it as stopped
                manager.processes[i].state = STOPPED;
            }
            break;
        }
    }

    // Reset the foreground process ID since there is no active foreground process
    foreground_pid = -1;
}

// Changes the state of a stopped background process to running (in the background)
int background_command(char* pid_str) {
    if (pid_str == NULL) {
        // If no PID is provided, print usage instructions
        printf("Usage: bg <pid>\n");
        return -1;
    }

    pid_t pid = atoi(pid_str);
    if (pid <= 0) {
        printf("Invalid PID\n");
        return -1;
    }

    // Search for the process in the manager's process list
    int found = -1;
    for (int i = 0; i < manager.count; i++) {
        if (manager.processes[i].pid == pid) {
            found = i;
            break;
        }
    }

    if (found == -1) {
        printf("No such process found\n");
        return -1;
    }

    // Get the process information from the process manager
    Process* proc = &manager.processes[found];

    // If the process is terminated, it can't be resumed
    if (proc->state == TERMINATED) {
        printf("No such process found\n");
        return -1;
    }

    // If the process is stopped, send a SIGCONT signal to resume it in the background
    else if (proc->state == STOPPED) {
        if (kill(-pid, SIGCONT) == -1) {
            return handle_error("kill() error");
        }

        // Update the process state to running
        proc->state = RUNNING;
        printf("Changed state of process %d to Running (in the background)\n", pid);
    }
    
    else {
        printf("Process %d is already running\n", pid);
    }
}
