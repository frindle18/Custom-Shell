# Custom Shell

A powerful Unix-like shell implementation in C, developed as a course project for CS3.301 - Operating Systems and Networks.

## Table of Contents

1. [Introduction](#introduction)
2. [Features](#features)
3. [Getting Started](#getting-started)
   - [Prerequisites](#prerequisites)
   - [Installation](#installation)
   - [Running the Shell](#running-the-shell)
4. [Usage](#usage)
   - [Custom Commands](#custom-commands)
   - [System Commands](#system-commands)
   - [I/O Operations](#io-operations)
   - [Process Management](#process-management)
   - [Networking](#networking)
5. [Advanced Features](#advanced-features)
6. [Assumptions](#assumptions)

## Introduction

This custom shell project implements a feature-rich Unix-like shell in C. It supports various custom commands, system commands, background processes, logging, I/O redirection, piping, and more. The shell provides a user-friendly interface with a custom prompt and color-coded output for enhanced usability.

## Features

- **Custom Prompt**: Displays username, system name, and current working directory
- **Multiple Command Execution**: Supports sequential and background execution
- **Custom Commands**: Includes `hop`, `reveal`, `log`, `proclore`, `seek`, `activities`, `ping`, `fg`, `bg`, `neonate`
- **System Command Execution**: Supports both foreground and background processes
- **I/O Redirection and Piping**: Enables complex command chaining and data manipulation
- **Process Management**: Provides tools for monitoring and controlling processes
- **Command History**: Logs and manages command history for easy recall and execution
- **Networking Capabilities**: Fetches man pages from the internet

## Getting Started

### Prerequisites

- GCC Compiler
- Make utility
- Unix-like operating system (Linux, macOS)

### Installation

1. Clone the repository:
   ```
   git clone https://github.com/frindle18/Custom-Shell
   cd Custom-Shell
   ```

2. Compile the shell:
   ```
   make
   ```

### Running the Shell

After compilation, start the shell with:

```
./shell
```

## Usage

### Custom Commands

1. **hop**: Change the current working directory
   ```
   hop <directory>
   ```
   - Supports `.`, `..`, `~`, and `-` as special arguments

2. **reveal**: List files and directories with color-coded output
   ```
   reveal [options] [directory]
   ```
   - Options: `-a` (show all), `-l` (detailed view)

3. **log**: Manage command history
   ```
   log
   log purge
   log execute <index>
   ```

4. **proclore**: Display process information
   ```
   proclore [pid]
   ```

5. **seek**: Search for files or directories
   ```
   seek [options] [search_term] [target_directory]
   ```
   - Options: `-d` (directories only), `-f` (files only), `-e` (execute/change directory)

6. **activities**: List all processes spawned by the shell
   ```
   activities
   ```

7. **ping**: Send signals to processes
   ```
   ping <pid> <signal_number>
   ```

8. **neonate**: Print the PID of the most recently created process
   ```
   neonate -n <interval_seconds>
   ```

### System Commands

Execute standard system commands in foreground or background:

```
command [args]    # Foreground
command [args] &  # Background
```

### I/O Operations

1. **I/O Redirection**:
   ```
   command > output.txt   # Output redirection (overwrite)
   command >> output.txt  # Output redirection (append)
   command < input.txt    # Input redirection
   ```

2. **Pipes**:
   ```
   command1 | command2 | command3
   ```

### Process Management

- **Foreground Process Control**:
  ```
  fg <pid>
  ```

- **Background Process Control**:
  ```
  bg <pid>
  ```

### Networking

- **iMan**: Fetch man pages from the internet
  ```
  iMan <command_name>
  ```

## Advanced Features

- **Aliases**: Define custom aliases in `.myshrc` configuration file
- **Multiple Command Execution**: Use `;` for sequential execution and `&` for background execution
- **Signal Handling**: Support for `Ctrl-C`, `Ctrl-D`, and `Ctrl-Z`

## Assumptions

- Paths and filenames should not contain whitespace characters
- The `seek` command's target directory should always start with `.`
- Commands from "log execute" are not added to the command history
- Aliases do not support multiple processes
- Functions can have commands with varying numbers of arguments and will still work correctly

---

Developed as part of CS3.301 - Operating Systems and Networks course project.
