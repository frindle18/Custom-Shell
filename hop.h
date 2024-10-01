#ifndef HOP_H
#define HOP_H

// Prints the current working directory
// Returns 0 if no errors, else -1
int print_cwd(void);

// Replaces the "~" of a user-inputed path with the path of the home directory
// input_dir is passed as an argument to the function and gets filled after execution
void handle_home_paths(char input_dir[], char home_dir[]);

// Implements a single change of directory using chdir()
// Returns -1 in case of any error, else returns 0
int change_dir(char dir[], char prev_dir[], char home_dir[]);

// Scans the hop command and makes calls to change_dir after tokenizing the input (for sequential hop)
// Returns -1 in case of any error, else returns 0
int hop(char command[], char prev_dir[], char home_dir[]);

#endif // HOP_H
