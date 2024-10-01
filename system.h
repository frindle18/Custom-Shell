#ifndef SYSTEM_H
#define SYSTEM_H

#include <sys/types.h>

#define MAX_PROCESSES 128

typedef enum ProcessState {
    RUNNING,
    STOPPED,
    TERMINATED
} ProcessState;

typedef struct Process {
    pid_t pid;
    char* command;
    ProcessState state;
} Process;

typedef struct ProcessManager {
    Process processes[MAX_PROCESSES];
    int count;
} ProcessManager;

extern ProcessManager manager;
extern pid_t foreground_pid;
extern pid_t shell_pgid;

// Executes a shell command, and also calculates the time taken to do so
// The parameter genius informs the function if the command is the last individual command in a piped command
long execute_shell_command(char command[], char command_type, char* args[], int genius);

#endif // SYSTEM_H
