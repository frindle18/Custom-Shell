#ifndef REVEAL_H
#define REVEAL_H

#include <dirent.h>
#include <sys/types.h>

#define MAX_SIZE 80

typedef enum Flags {
    a,
    l
} Flags;

// Gets the flags and the required directory from the command
// Assumption: Only one target directory will be considered, i.e., the part of the input command after all the flags and the first directory entered will not be considered
// Array flags and string target_dir passed as arguments to the function will be filled after execution
// Returns -1 if there are invalid flags, returns 0 if function is succesfull
int parse_reveal(char command[], int flags[], char target_dir[]);

// Gets the real path of the required directory (including flags like "~" and "-") and puts it in target_dir which is passed as an argument
// Returns -1 in case of any error, else returns 0
int get_target_directory_path(char input_dir[], char prev_dir[], char home_dir[], char target_dir[]);

// Prints the color coded name of given entry
// Assumption: Files in green, directories in blue with green background, executables in red
void print_color_coded_name(struct dirent* entry, mode_t mode);

// Assumption: We skip over entries that cause an error in the stat() function
void print_entry(struct dirent* entry, char target_dir[]);

// Prints file permissions the same way the ls -l command does
void print_file_perms(mode_t mode, char perms[]);

// Assumption: We skip over entries that cause an error in the stat() function
void print_entry_details(struct dirent* entry, char target_dir[]);

// Calculates and returns block size (which is output in the ls -l command)
int get_block_size(struct dirent* entry, char target_dir[]);

// Loops over all entries in specified target directory and calls the above functions as per the flags
// Returns -1 in case of any error, else returns 0
int reveal(char command[], char prev_dir[], char home_dir[]);

#endif // REVEAL_H
