#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "activities.h"
#include "input.h" // void remove_spaces(char* command)
#include "prompt.h" // Color codes
#include "system.h" // struct Process

// Compares two processes by their command name (used in sorting)
int compare_processes(const void* a, const void* b) {
    Process* procA = (Process*)a;
    Process* procB = (Process*)b;
    return strcmp(procA->command, procB->command);
}

// Displays a list of processes sorted by their command name
void activities(void) {
    if (manager.count == 0) {
        printf("No active processes.\n");
        return;
    }

    Process* sorted_processes = malloc(manager.count * sizeof(Process));

    memcpy(sorted_processes, manager.processes, manager.count * sizeof(Process));

    qsort(sorted_processes, manager.count, sizeof(Process), compare_processes);

    for (int i = 0; i < manager.count; i++) {
        char* state_str;

        switch (sorted_processes[i].state) {
            case RUNNING:
                state_str = "Running";
                break;
            case STOPPED:
                state_str = "Stopped";
                break;
            case TERMINATED:
                state_str = "Terminated";
                break;
            default:
                state_str = "Unknown";
        }

        remove_spaces(sorted_processes[i].command);
        
        // Print the process information: PID, command, and state
        printf(CYAN "%d" RESET " : %s - " MAGENTA "%s\n" RESET, sorted_processes[i].pid, sorted_processes[i].command, state_str);
    }

    free(sorted_processes);
}
