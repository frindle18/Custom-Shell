#ifndef ACTIVITIES_H
#define ACTIVITIES_H

// Compares two processes by their command name (used in sorting)
int compare_processes(const void* a, const void* b);

// Displays a list of processes sorted by their command name
void activities(void);

#endif // ACTIVITIES_H
