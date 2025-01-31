/**
 * Author: Jian Long Ye (Noah)
 * Date: 2023-10-10
 * Decription: simple shell program that coantians built in commands, output
 * redirection, ability to handle background and foreground processes and basic
 * pipe lining
 *
 *
 * NOTE: Assume in output redirection the argument after > is always 1 for
 * example just 1 text file, cannot have ls > arg args.
 *
 * NOTE: Also assume that user won't make any spelling errors and have correct
 * spacing for each command
 * 
 * NOTE: Fg with no id will always take job id 1
 *
 * NOTE: Assume a maximum of 100 processes
 *
 */

// DISCLAIMER: This code is given for illustration purposes only. It can contain
// bugs! You are given the permission to reuse portions of this code in your
// assignment.
//
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
//
// This code is given for illustration purposes. You need not include or follow
// this strictly. Feel free to write better or bug free code. This example code
// block does not worry about deallocating memory. You need to ensure memory is
// allocated and deallocated properly so that your shell works without leaking
// memory.
//

int getcmd(char *prompt, char *args[], int *background, int *redirect,
           int *isPipe);
void echoCommand(char **phrase, int token);
void cdCommand(char *directory);
void pwdCommand();
void jobCommand();
void fgCommand(int jobId); //Fg with no id will always take job id 1
void exitCommand();
void outRedirect(char **args, int cnt);
void pipping(char *args[20], int cnt);
void removeCompletedJobs();
struct Processes {
  int id;       // id for to bring certain jobs to foreground with fg
  int pid;      // return process id of process
  char *status; // background running or not
  int isBg;     // background or foreground
  char *params;
};
struct Processes
    processesArray[101]; // assume there will not be more than 100 proccesses
int processCount = 0;    // track the number of background proccess

int main(void) {
  char *args[20];
  int bg;
  int redirect; // check if user is doing output redirec
  int isPipe;   // check if user is doing pipping

  while (1) {
    // initialize all values to false (0)
    bg = 0;
    redirect = 0;
    isPipe = 0;
    int cnt = getcmd("\n>> ", args, &bg, &redirect, &isPipe);
    removeCompletedJobs();

    // All built in commands in the handout
    if (cnt > 0) { // make sure there is at lease 1 argument
      if (strcmp(args[0], "echo") == 0) {
        echoCommand(args, cnt);
      } else if (strcmp(args[0], "cd") == 0) {
        if (cnt > 1) {
          cdCommand(args[1]);
        } else {
          printf("Input a directory\n");
        }
      } else if (strcmp(args[0], "pwd") == 0) {
        if (cnt == 1) {
          pwdCommand();
        } else {
          printf("pwd does not need additonal argumnents");
        }
      } else if (strcmp(args[0], "exit") == 0) {
        if (cnt == 1) {
          exitCommand();
        } else {
          printf("exit does not need additonal argumnents");
        }
      } else if (strcmp(args[0], "jobs") == 0) {

        if (cnt == 1) {
          jobCommand();
        } else {
          printf("jobs does not need additonal argumnents");
        }
      } else if (strcmp(args[0], "fg") == 0) {
        // when user just type fg bring the first task to foreground
        if (cnt == 1) {
          fgCommand(1);
        }

        else {
          // convert to int
          int jobId = atoi(args[1]);
          fgCommand(jobId);
        }
      }

      else {
        int pid = fork();

        if (pid < 0) {
          printf("pid is less than 0");
          exit(1);
        } else if (pid == 0) { // Child process
          // output redirection
          if (redirect == 1) {
            printf("redirect");
            outRedirect(args, cnt);
          }
          // for command pipping
          else if (isPipe == 1) {

            pipping(args, cnt);
          }
          // general terminal commands
          else {
            args[cnt] = NULL;
            execvp(args[0], args);
            perror("execvp for general commands");
            exit(1);
          }
        } else { // Parent process

          int status;
          if (bg == 0) {
            // Wait for foreground process
            waitpid(pid, &status, 0);
          } else {

            if (processCount < 16) {

              // Store background process information in the next available slot
              processesArray[processCount].pid = pid;
              processesArray[processCount].isBg = bg;
              processesArray[processCount].params =
                  (char *)malloc(10);         // Adjust size as needed
              for (int i = 0; i < cnt; i++) { // to show what the command is for
                                              // fg when a process is finshed
                strcat(processesArray[processCount].params, args[i]);
                strcat(processesArray[processCount].params, " ");
              }
              processCount++;
            }
          }
        }
      }
    } else {
      printf("please input at least 1 commmand");
    }
  }
}

int getcmd(char *prompt, char *args[], int *background, int *redirect,
           int *isPipe) {
  int length, i = 0;
  char *token, *loc;
  char *line = NULL;
  size_t linecap = 0;
  printf("%s", prompt);
  length = getline(&line, &linecap, stdin);

  if (length <= 0) {
    exit(-1);
  }

  // check for output rediretion
  if ((loc = index(line, '>')) != NULL) {
    *redirect = 1;
  }

  // check for pipe 
  else if ((loc = index(line, '|')) != NULL) {
    *isPipe = 1;
  }

  // Check if background is specified..
  else if ((loc = index(line, '&')) != NULL) {
    *background = 1;
    *loc = ' ';
  } else
    *background = 0;
  while ((token = strsep(&line, " \t\n")) != NULL) {
    for (int j = 0; j < strlen(token); j++)
      if (token[j] <= 32)
        token[j] = '\0';
    if (strlen(token) > 0)
      args[i++] = token;
  }
  return i;
}

void echoCommand(char **phrase, int token) {
  for (int i = 1; i < token; i++) {
    printf("%s ", phrase[i]);
  }
}

void cdCommand(char *directory) {
  const char *nextDirec = directory;

  if (chdir(nextDirec) == 0) {
    printf("Changed directory to %s\n", nextDirec);
  } else {
    printf("Wrong directory\n");
  }
}

void pwdCommand() {
  char *buffer = (char *)malloc(100);

  if (getcwd(buffer, 1000) != NULL) {

    printf("Current working dir: %s\n", buffer);
    free(buffer);
  }
}

// Note: assuming there will not be 100 proceces and need to remove the last
// element since i+1 would be out of bounds

void jobCommand() {
  for (int i = 0; i < processCount; i++) {
    processesArray[i].id = i + 1;
    printf("Job ID: %d\n", processesArray[i].id);
    printf("Process ID (PID): %d\n", processesArray[i].pid);
    printf("Process enviroment: %s\n",
           (processesArray[i].isBg == 1) ? "background" : "foreground");
    printf("Process Status: %s\n", processesArray[i].status);
    printf("\n");
  }
}

void fgCommand(int jobId) {
  int found = 0;
  int processIndex = 0;
  for (int i = 0; i < processCount; i++) {
    processesArray[i].id = i + 1;
  }

  for (int i = 0; i < processCount; i++) {
    if (processesArray[i].id == jobId) {
      found = 1;
      processIndex = i;
      break; // Exit the loop once the process is found
    }
  }

  if (found) {

    int status;
    int pid = processesArray[processIndex].pid;
    waitpid(pid, &status, WUNTRACED); // Wait for the process to finish or be stopped
    printf(" %s", processesArray[processIndex].params);
    free(processesArray[processIndex].params);

    // Remove the process from the list of background processes
    for (int j = processIndex; j < processCount - 1; j++) {
      processesArray[j] = processesArray[j + 1];
    }
    processCount--;
  } else {
    printf("Job with ID %d not found in the background.\n", jobId);
  }
}

void exitCommand() {
  for (int i = 0; i < processCount; i++) {
    kill(processesArray[i].pid, SIGKILL);
  }
  printf("Terminated\n");
  exit(0);
}

void outRedirect(char **args, int cnt) {

  int size = 0; // Initialize size to 0 as a default value

  for (int i = 0; i < cnt; i++) {
    if (strcmp(args[i], ">") == 0) {
      size = i; // Set size to the index where ">" was found
      break;
    }
  }

  char *inputArray[size + 1]; // +1 for NULL terminator

  for (int i = 0; i < size; i++) {
    inputArray[i] = args[i];
  }

  inputArray[size] = NULL; // Null-terminate the inputArray

  int fd = open(args[cnt - 1], O_WRONLY | O_CREAT | O_TRUNC, 0777);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  int fd2 = dup2(fd, 1); // Redirect stdout to the file descriptor
  close(fd);

  if (fd2 < 0) {
    perror("dup2");
    exit(1);
  }

  execvp(inputArray[0], inputArray); // Execute the command with redirection
  perror("execvp");
  exit(1);
}

void pipping(char *args[20], int cnt) {
  int inputSize = 0;
  int outSize = 0;

  for (int i = 0; i < cnt; i++) {
    if (strcmp(args[i], "|") == 0) { // only piping has the | symbol
      inputSize = i;
      break;
    }
  }

  outSize = cnt - inputSize;

  // for storing the output and input arguments
  char *input[inputSize + 1]; // this is for execvp() to set last element to null
  char *out[outSize];

  for (int i = 0; i < inputSize; i++) {
    input[i] = args[i];
  }

  int j = 0;
  for (int i = inputSize + 1; i < cnt; i++) {

    out[j] = args[i];
    j++;
  }

  input[inputSize] = NULL;
  out[outSize - 1] = NULL;

  // setting up the pip
  int fdPipe[2];
  if (pipe(fdPipe) < 0) {
    perror("pipe creation");
  }

  int outputForkId = fork();

  if (outputForkId == -1) {
    printf("The value of the output redirection fork is -1");
  }

  // starting output redirection
  else if (outputForkId == 0) {

    if (dup2(fdPipe[1], 1) < 0) {
      perror("dup2 for input fork");
    }

    close(fdPipe[0]);
    // input are arguments before |
    execvp(input[0], input);
    perror("execvp in output redirection");
  }

  else {
    waitpid(outputForkId, NULL, 0);
    int inputForkId = fork();

    if (inputForkId == -1) {
      printf("The value of the input redirection fork is -1");
    }

    // starting input redirection
    else if (inputForkId) {

      if (dup2(fdPipe[0], 0) < 0) {
        perror("dup2 for input fork");
      }

      close(fdPipe[1]);
      // out are arguments after the |
      execvp(out[0], out);
      perror("execvp in input redirection");
    }

    else {

      waitpid(inputForkId, NULL, 0);
      close(fdPipe[1]);
    }
  }
}

// Function to remove completed jobs from the list
void removeCompletedJobs() {
  int i = 0;
  while (i < processCount) {
    if (waitpid(processesArray[i].pid, NULL, WNOHANG) > 0) {
      // Job has completed

      // Free the memory for the 'params' string
      free(processesArray[i].params);

      for (int j = i; j < processCount - 1; j++) {
        // Shift the remaining jobs in the array
        processesArray[j] = processesArray[j + 1];
      }
      processCount--;
    } else {
      processesArray[i].status = "running";
      i++;
    }
  }
}
