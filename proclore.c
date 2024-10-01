#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "proclore.h"
#include "prompt.h" // #define MAX_PATH_SIZE 4096

// Prints details of the process as required by proclore
void get_process_details(int pid) {
    char path[MAX_PATH_SIZE]; // File path to files in the /proc filesystem

    FILE* file; // File pointer to the above file

    int group_id, vm_size; // Process group id, virtual memory size (in kB)
    char state; // State of the process
    char exe_path[MAX_PATH_SIZE]; // Path to executable file of process
    char process_type = ' '; // Foreground or background process

    // If pid is 0 or not provided, use the shell's pid
    if (pid == 0) {
        pid_t shell_pid = getpid();
        pid = shell_pid;
    }

    // Read process state and process group from /proc/[pid]/stat
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    file = fopen(path, "r");
    if (file) {
        fscanf(file, "%*d %*s %c %d", &state, &group_id); // Discard the first %d and %s, indicated by *
        fclose(file);
    }

    // Check if the process is in the foreground or background
    if (tcgetpgrp(STDIN_FILENO) == group_id) {
        process_type = '+';
    }

    // Read virtual memory size from /proc/[pid]/status
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    file = fopen(path, "r");
    if (file) {
        char buffer[READ_SIZE];
        while (fgets(buffer, sizeof(buffer), file)) {
            if (sscanf(buffer, "VmSize: %d", &vm_size) == 1) { // Returns 1 if it matches the line "Vmsize: %d", and stores the %d into vm_size
                break;
            }
        }
        fclose(file);
    }

    // Get executable path from /proc/[pid]/exe
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    ssize_t len = readlink(path, exe_path, sizeof(exe_path) - 1);
    if (len != -1) {
        exe_path[len] = '\0';
    }

    // Print the collected information
    printf("PID : %d\n", pid);
    printf("Process Status : %c%c\n", state, process_type);
    printf("Process Group : %d\n", group_id);
    printf("Virtual Memory : %d\n", vm_size);
    printf("Executable Path : %s\n", exe_path);
}

// Called by main for handling proclore() command
// Assumption: The input is always of the form "proclore" or "proclore %d"
void proclore(char command[]) {
    int pid = 0;

    if (strcmp(command, "proclore") != 0) {
        pid = atoi(command + strlen("proclore "));
    }

    get_process_details(pid);
}
