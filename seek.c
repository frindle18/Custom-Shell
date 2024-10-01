#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hop.h" // void handle_home_paths(char input_dir[], char home_dir[])
#include "prompt.h" // #define MAX_PATH_SIZE 4096
#include "seek.h"

// Displays contents of a file on the terminal
int display_file_contents(char filename[]) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        return handle_error("fopen() error");
    }

    char ch;
    while ((ch = fgetc(file)) != EOF) {
        putchar(ch);
    }

    fclose(file);

    return 0;
}

// Allocates memory for and returns a new match
Match* create_match(char path[], char type[]) {
    Match* new_match = (Match*)malloc(sizeof(Match));

    strcpy(new_match->path, path);
    strcpy(new_match->type, type);
    new_match->next = NULL;

    return new_match;
}

// Adds a new match to the list of matches with head head
void add_match(Match** head, char path[], char type[]) {
    Match* new_match = create_match(path, type);

    if (*head == NULL) {
        *head = new_match;
    }
    
    else {
        Match* temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_match;
    }
}

// Parse the command to get flags, target name, and the target directory
// Assumption: The target directory always starts with a "./"
// Returns -1 in case of any error, else returns 0
int parse_seek(char command[], int flags[], char target_name[], char target_dir[]) {
    char* token = strtok(command, " ");

    while (token != NULL) {
        if (token[0] == '-') {
            int n = strlen(token);
            for (int i = 1; i < n; i++) {
                if (token[i] == 'd') {
                    flags[d] = 1;
                }
                else if (token[i] == 'f') {
                    flags[f] = 1;
                }
                else if (token[i] == 'e') {
                    flags[e] = 1;
                }
                else {
                    printf("Invalid flag\n");
                    return -1;
                }
            }
        }
        
        else if (token[0] == '.' || token[0] == '~') {
            strcpy(target_dir, token); // Target directory
        }

        else if (strcmp(token, "seek") != 0) {
            strcpy(target_name, token); // Target name
        }

        token = strtok(NULL, " ");
    }

    return 0;
}

// Compares each entry name to the target name, and recursively searches all the directories
// Appends matches to Match** results, a list of matches
// Assumption: We skip over entries that cause an error in the stat() function
int search_directory(int flags[3], char target_name[], char target_dir[], Match** results) {
    DIR* dir;

    if ((dir = opendir(target_dir)) == NULL) {
        return handle_error("opendir() error");
    }

    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char path[MAX_PATH_SIZE];

        // Construct full path of each entry
        snprintf(path, sizeof(path), "%s/%s", target_dir, entry->d_name);

        struct stat entry_stat;

        // Get the entry's stat information
        if (stat(path, &entry_stat) == -1) {
            fprintf(stderr, "%s: %s\n", "stat() error", strerror(errno));
            continue;
        }

        // If it's a directory
        if (S_ISDIR(entry_stat.st_mode)) {
            if (strncmp(entry->d_name, target_name, strlen(target_name)) == 0) {
                add_match(results, path, "directory");
            }
            // Recurse into the directory
            if (search_directory(flags, target_name, path, results) == -1) {
                return -1;
            }
        }

        // If it's a regular file
        else if (S_ISREG(entry_stat.st_mode)) {
            if (strncmp(entry->d_name, target_name, strlen(target_name)) == 0) {
                add_match(results, path, "file");
            }
        }
    }

    closedir(dir);

    return 0;
}

// Prints color coded match, files in cyan and directories in magenta
void print_match(char type[], char path[], char target_dir[]) {
    if (strcmp(type, "file") == 0) {
        printf(CYAN ".%s\n" RESET, path + strlen(target_dir));
    }
    else if (strcmp(type, "directory") == 0) {
        printf(MAGENTA ".%s\n" RESET, path + strlen(target_dir));
    }
}

// Called by main to handle the "seek" command
// Assumption: -e flag is used only in combination with -d or -f flags
// Returns -2 in case of invalid flags, -1 in case of other errors, else returns 0
int seek(char command[], char home_dir[]) {
    int flags[3] = {0, 0, 0};
    char target_name[MAX_NAME_SIZE];
    char target_dir[MAX_PATH_SIZE] = "."; // If no target_dir, seek in current directory

    // Parse the command to get flags, target name and target directory
    parse_seek(command, flags, target_name, target_dir);

    if (target_dir[0] == '~') {
        handle_home_paths(target_dir, home_dir);
    }

    if (flags[d] && flags[f]) {
        printf("Invalid flags\n");
        return -2;
    }

    Match* head = NULL;

    if (search_directory(flags, target_name, target_dir, &head) == -1) {
        return -1;
    }

    // head now contains a list of all matches

    int file_count = 0, dir_count = 0;
    char file_path[MAX_PATH_SIZE], dir_path[MAX_PATH_SIZE];

    Match* temp = head;

    while (temp != NULL) {
        if (strcmp(temp->type, "file") == 0) {
            if (flags[d] == 0) {
                print_match(temp->type, temp->path, target_dir);
            }
            file_count++;
            if (file_count == 1) { // // Keep track of file path if there's only one match
                strcpy(file_path, temp->path);
            }
        }
        else if (strcmp(temp->type, "directory") == 0) {
            if (flags[f] == 0) {
                print_match(temp->type, temp->path, target_dir);
            }
            dir_count++;
            if (dir_count == 1) { // Keep track of directory path if there's only one match
                strcpy(dir_path, temp->path);
            }
        }
        temp = temp->next;
    }
   
    // If there is a single match and -e flag is used
    if (flags[e] == 1) {
        // -de flag -> Change into directory found
        if (flags[d] == 1 && file_count == 1) {
            if (chdir(dir_path) == -1) {
                return handle_error("chdir() error");
            }
        }

        // -fe flag -> Print contents of file found
        if (flags[f] == 1 && dir_count == 1) {
            if (display_file_contents(file_path) == -1) {
                return -1;
            }
            printf("\n");
        }
    }

    return 0;
}
