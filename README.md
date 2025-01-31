# Simple Shell

## Author: Jian Long Ye (Noah)
## Date: October 10, 2023

## Description
This project is a simple shell program that supports built-in commands, output redirection, background and foreground process handling, and basic pipelining.

## Features
- **Built-in Commands:**
  - `echo`: Prints the given arguments.
  - `cd <directory>`: Changes the current working directory.
  - `pwd`: Prints the current working directory.
  - `exit`: Terminates the shell and kills any background processes.
  - `jobs`: Lists background processes.
  - `fg <jobId>`: Brings a background process to the foreground. If no ID is given, it defaults to job ID 1.
- **Output Redirection:**
  - Supports `>` for redirecting output to a file.
  - Example: `ls > output.txt`
- **Process Handling:**
  - Supports running commands in the background using `&`.
  - Example: `sleep 10 &`
  - Tracks background processes and their statuses.
- **Pipelining:**
  - Supports simple pipelining using `|`.
  - Example: `ls | grep .c`

## Assumptions
- Output redirection is always followed by a single file argument.
- Users provide correct command syntax (no spelling mistakes or incorrect spacing).
- `fg` without an argument defaults to job ID 1.
- A maximum of 100 background processes can be tracked at a time.

## Compilation & Execution
1. Compile the program using:
   ```bash
   gcc -o shell shell.c
