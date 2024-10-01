#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "hop.h" // void handle_home_paths(char input_dir[], char home_dir[])
#include "prompt.h" // int get_dir(char dir[])
#include "reveal.h"

// Gets the flags and the required directory from the command
// Assumption: Only one target directory will be considered, i.e., the part of the input command after all the flags and the first directory entered will not be considered
// Array flags and string target_dir passed as arguments to the function will be filled after execution
// Returns -1 if there are invalid flags, returns 0 if function is succesfull
int parse_reveal(char command[], int flags[], char target_dir[]) {
    char* token = strtok(command, " ");

    while (token != NULL) {
        if (token[0] == '-' && strlen(token) > 1) { // Avoid - flag which signifies previous directory
            int n = strlen(token);
            for (int i = 1; i < n; i++) {
                if (token[i] == 'a') {
                    flags[a] = 1;
                }
                else if (token[i] == 'l') {
                    flags[l] = 1;
                }
                else {
                    printf("Invalid flag\n");
                    return -1;
                }
            }
        }

        else if (strcmp(token, "reveal") != 0) {
            strcpy(target_dir, token);
            break;
        }

        token = strtok(NULL, " ");
    }

    return 0;
}

// Gets the real path of the required directory (including flags like "~" and "-") and puts it in target_dir which is passed as an argument
// Returns -1 in case of any error, else returns 0
int get_target_directory_path(char input_dir[], char prev_dir[], char home_dir[], char target_dir[]) {
    // Empty paths
    if (input_dir[0] == '\0') {
        if (get_dir(input_dir) == -1) {
            return -1;
        }
    } // If there's no argument for reveal, then take current directory
    
    // Paths starting with ~
    else if (input_dir[0] == '~') {
        handle_home_paths(input_dir, home_dir);
    }

    // Reveal contents of previous directory
    if (strcmp(input_dir, "-") == 0) {
        strcpy(input_dir, prev_dir);
    }

    if (realpath(input_dir, target_dir) == NULL) {
        return handle_error("realpath() error");
    }

    return 0;
}

// Prints the color coded name of given entry
// Assumption: Files in green, directories in blue with green background, executables in red
void print_color_coded_name(struct dirent* entry, mode_t mode) {
    if (S_ISDIR(mode)) { // Directory
        printf(BLUE BG_GREEN "%s" RESET "  ", entry->d_name);
    }

    else if (S_ISREG(mode) && (mode & (S_IXUSR | S_IXGRP | S_IXOTH))) { // Executable by file owner, group or others
        printf(RED "%s" RESET "  ", entry->d_name);
    }

    else if (S_ISREG(mode)) { // Regular file
        printf(GREEN "%s" RESET "  ", entry->d_name);
    }

    printf("\n");
}
 
// Assumption: We skip over entries that cause an error in the stat() function
void print_entry(struct dirent* entry, char target_dir[]) {
    char entry_path[MAX_PATH_SIZE];
    snprintf(entry_path, sizeof(entry_path), "%s/%s", target_dir, entry->d_name); // entry_path has the absolute path which is needed for stat()

    struct stat entry_stat;

    if (stat(entry_path, &entry_stat) == -1) {
        fprintf(stderr, "%s: %s\n", "stat() error", strerror(errno));
        return; // Skip over entries that cause an error in the stat() function
    }

    mode_t mode = entry_stat.st_mode;

    print_color_coded_name(entry, mode);
}

// Prints file permissions the same way the ls -l command does
void print_file_perms(mode_t mode, char perms[]) {
    perms[0] = (S_ISDIR(mode)) ? 'd' : '-';

    // User permissions
    perms[1] = (mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (mode & S_IXUSR) ? 'x' : '-';

    // Group permissions
    perms[4] = (mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (mode & S_IXGRP) ? 'x' : '-';

    // Others permissions
    perms[7] = (mode & S_IROTH) ? 'r' : '-';
    perms[8] = (mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (mode & S_IXOTH) ? 'x' : '-';

    perms[10] = '\0';

    printf("%s ", perms);
}

// Assumption: We skip over entries that cause an error in the stat() function
void print_entry_details(struct dirent* entry, char target_dir[]) {
    char entry_path[MAX_PATH_SIZE];
    snprintf(entry_path, sizeof(entry_path), "%s/%s", target_dir, entry->d_name); // entry_path has the absolute path which is needed for stat()

    struct stat entry_stat;

    if (stat(entry_path, &entry_stat) == -1) {
        fprintf(stderr, "%s: %s\n", "stat() error", strerror(errno));
        return; // Skip over entries that cause an error in the stat() function
    }

    char perms[11];
    mode_t mode = entry_stat.st_mode;
    print_file_perms(mode, perms); // File permissions

    printf("%lu ", entry_stat.st_nlink); // Number of references or links to the entry

    struct passwd* pw = getpwuid(entry_stat.st_uid);
    printf("%s ", pw->pw_name); // Username of file owner

    gid_t group_id = entry_stat.st_gid; // Group id

    struct group *grp = getgrgid(group_id);
    printf("%s ", grp->gr_name); // Group name of file's group
            
    printf("%5ld ", entry_stat.st_size); // Size of file in bytes

    struct tm* time_info = localtime(&entry_stat.st_mtime);

    char time[MAX_SIZE];
    strftime(time, sizeof(time), "%b %d %H:%M", time_info); // Formatted like "Jan 01 00:00"
    printf("%s ", time);

    print_color_coded_name(entry, mode); // Color coded name
}

// Calculates and returns block size (which is output in the ls -l command)
int get_block_size(struct dirent* entry, char target_dir[]) {
    char entry_path[MAX_PATH_SIZE];
    snprintf(entry_path, sizeof(entry_path), "%s/%s", target_dir, entry->d_name); // entry_path has the absolute path which is needed for stat()

    struct stat entry_stat;

    if (stat(entry_path, &entry_stat) == -1) {
        return -1;
    }

    return entry_stat.st_blocks;
}

// Loops over all entries in specified target directory and calls the above functions as per the flags
// Returns -1 in case of any error, else returns 0
int reveal(char command[], char prev_dir[], char home_dir[]) {
    char input_dir[MAX_PATH_SIZE] = "";
    int flags[2] = {0, 0}; // flags[0] - a, flags[1] = l

    // Get directory and the flags for reveal command
    if (parse_reveal(command, flags, input_dir) == -1) {
        return -1;
    }

    char target_dir[MAX_PATH_SIZE];

    if (get_target_directory_path(input_dir, prev_dir, home_dir, target_dir) == -1) {
        return -1;
    }

    DIR* dir;

    if ((dir = opendir(target_dir)) == NULL) {
        return handle_error("opendir() error");
    }

    struct dirent* entry;

    // If flags[a] and flags[l] are both set, print total blocks
    if (flags[a] == 1 && flags[l] == 1) {
        int total_blocks = 0;

        while ((entry = readdir(dir)) != NULL) {
            total_blocks += get_block_size(entry, target_dir);
        }

        printf("total %d\n", total_blocks / 2); // Convert from 512-byte blocks reported by stat system call to 1-kilobyte blocks
    }

    rewinddir(dir); // Reset directory stream

    while ((entry = readdir(dir)) != NULL) {
        if (flags[a] == 0 && entry->d_name[0] == '.') {
            continue; // Skip hidden files if no -a flag
        }

        // If flag 'l' is set, print detailed info of each entry
        if (flags[l] == 1) {
            print_entry_details(entry, target_dir);
        }

        // Otherwise, print just names
        else {
            print_entry(entry, target_dir);
        }
    }

    closedir(dir);

    return 0;
}
