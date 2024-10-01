#ifndef FG_BG_H
#define FG_BG_H

// Brings the running or stopped background process with corresponding pid to foreground
int foreground_command(char* pid_str);

// Changes the state of a stopped background process to running (in the background)
int background_command(char* pid_str);

#endif // FG_BG_H
