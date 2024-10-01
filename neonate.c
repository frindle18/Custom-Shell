#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "neonate.h"
#include "prompt.h" // int handle_error(char* error_message)

// Global variable to store original terminal settings
static struct termios old_termios;

// Retrieve the most recent process ID (PID) from the /proc directory
// Returns the most recent PID found
int get_recent_pid(void) {
    DIR* proc_dir;

    struct dirent* entry;
    struct stat statbuf;

    char path[MAX_PATH_SIZE];
    time_t newest_time = 0;
    pid_t recent_pid = -1;

    proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        return handle_error("opendir() error");
    }

    while ((entry = readdir(proc_dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            int pid = atoi(entry->d_name);
            if (pid > 0) { // Ensuring it's a PID directory
                snprintf(path, sizeof(path), "/proc/%d", pid);
                if (stat(path, &statbuf) == 0) {
                    if (statbuf.st_ctime > newest_time) {
                        newest_time = statbuf.st_ctime;
                        recent_pid = pid;
                    }
                }
            }
        }
    }

    closedir(proc_dir);

    return recent_pid;
}

// Disable the signals for Ctrl+C (SIGINT) and Ctrl+Z (SIGTSTP)
// Returns -1 in case of errors, else returns 0
int disable_signals(void) {
    // Ignore SIGINT (Ctrl+C) and SIGTSTP (Ctrl+Z)
    if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
        return handle_error("signal() error");
    }

    if (signal(SIGTSTP, SIG_IGN) == SIG_ERR) {
        return handle_error("signal() error");
    }

    return 0;
}

// Set terminal to non-canonical mode to read input character by character
// Returns -1 in case of errors, else returns 0
int set_nonblocking_input(void) {
    struct termios new_termios;

    // Get current terminal settings
    if (tcgetattr(STDIN_FILENO, &old_termios) == -1) {
        return handle_error("tcgetattr() error");
    }

    // Copy settings to modify
    new_termios = old_termios;

    // Disable canonical mode and echo
    new_termios.c_lflag &= ~(ICANON | ECHO);

    // Set minimum number of input bytes read() should wait for
    new_termios.c_cc[VMIN] = 1;
    new_termios.c_cc[VTIME] = 0;

    // Apply new settings
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_termios) == -1) {
        return handle_error("tcgetattr() error");
    }

    return 0;
}

// Restore original terminal settings
// Returns -1 in case of errors, else returns 0
int reset_terminal_settings(void) {
    if (tcsetattr(STDIN_FILENO, TCSANOW, &old_termios) == -1) {
        return handle_error("tcgetattr() error");
    }

    return 0;
}

// Print the most recent PID at specified intervals until 'x' or Ctrl+D is pressed
// Returns -1 in case of errors, else returns 0
int print_recent_pid(int time_arg) {
    disable_signals(); // Disable Ctrl+C and Ctrl+Z signals
    set_nonblocking_input(); // Set terminal to non-canonical mode

    char input; // Variable to store user input

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);

        struct timeval tv;
        tv.tv_sec = time_arg;
        tv.tv_usec = 0;

        int retval = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

        if (retval == -1) {
            return handle_error("select() error");
        }
        
        else if (retval > 0) {
            // Data is available to read
            ssize_t bytes_read = read(STDIN_FILENO, &input, 1);
            if (bytes_read > 0) {
                if (input == 'x') {
                    printf("Exiting...\n");
                    break;
                }
                
                else if (input == '\x04') { // Check for Ctrl+D (ASCII code 4)
                    reset_terminal_settings(); // Restore terminal settings before exiting
                    exit(0);
                }

            }
            
            else if (bytes_read < 0) {
                return handle_error("read() error");
            } 
        }
        
        int recent_pid = get_recent_pid();
        if (recent_pid != -1) {
            printf("Most recent PID: %d\n", recent_pid);
        }
    }

    reset_terminal_settings(); // Restore terminal settings

    return 0;
}

// Function which handles the neonate command by calling the above implemented functions
// Returns 0 if no errors, else -1
int neonate(char* argv[]) {
    // Check if the first argument is the '-n' flag
    if (strcmp(argv[1], "-n") != 0) {
        printf("Error: Unknown flag. Usage: %s -n [time_arg]\n", argv[0]);
        return -1;
    }

    // Parse the time argument from the second argument
    int time_arg = atoi(argv[2]);  // Convert the time argument to an integer
    if (time_arg <= 0) { // If the time argument is not a positive integer
        printf("Error: time_arg must be a positive integer\n");
        return -1;
    }

    // Print the most recent PID at specified intervals
    print_recent_pid(time_arg);

    return 0;
}
