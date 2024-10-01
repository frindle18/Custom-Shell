#ifndef SEEK_H
#define SEEK_H

typedef struct Match {
    char path[MAX_PATH_SIZE];
    char type[20]; // Whether it's a file or directory
    struct Match* next;
} Match;

typedef enum SeekFlags {
    d,
    f,
    e
} SeekFlags;

// Displays contents of a file on the terminal
int display_file_contents(char filename[]);

// Allocates memory for and returns a new match
Match* create_match(char path[], char type[]);

// Adds a new match to the list of matches with head head
void add_match(Match** head, char path[], char type[]);

// Parse the command to get flags, target name, and the target directory
// Assumption: The target directory always starts with a "./"
// Returns -1 in case of any error, else returns 0
int parse_seek(char command[], int flags[], char target_name[], char target_dir[]);

// Compares each entry name to the target name, and recursively searches all the directories
// Appends matches to Match** results, a list of matches
// Assumption: We skip over entries that cause an error in the stat() function
int search_directory(int flags[3], char target_name[], char target_dir[], Match** results);

// Prints color coded match, files in cyan and directories in magenta
void print_match(char type[], char path[], char target_dir[]);

// Called by main to handle the "seek" command
// Returns -2 in case of invalid flags, -1 in case of other errors, else returns 0
int seek(char command[], char home_dir[]);

#endif // SEEK_H
