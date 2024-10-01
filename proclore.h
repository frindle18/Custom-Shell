#ifndef PROCLORE_H
#define PROCLORE_H

#define READ_SIZE 1024

// Prints details of the process as required by proclore
void get_process_details(int pid);

// Called by main for handling proclore() command
// Assumption: The input is always of the form "proclore" or "proclore %d"
void proclore(char command[]);

#endif // PROCLORE_H
