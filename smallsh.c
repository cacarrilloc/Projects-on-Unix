/***************************************************************************
 **                                                                        *
 ** Author:      Carlos Carrillo-Calderon                                  *
 ** Date:        07/28/16                                                  *
 ** Filename:    csmallsh.c                                                *
 **                                                                        *
 ** Description: This program simulates a shell, called smallsh. The shell *
 **              prompts for a command line and running commands. Also, it *
 **              allows the redirection of standard input and standard     *
 **              output and supports both foreground and background        *
 **              processes. Additionally, the shell support three built in *
 **              commands: exit, cd, and status, as well as comments,      *
 **              which are lines beginning with the # character.           *
 **                                                                        *
 **************************************************************************/

// Define Libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#define SIZE 513

// Function Prototypes/definitions.
void killProcess(void);
char *read_input(void);
void printStatus(int status);
void finishProcesses(pid_t pid);
void tokenize(char* input, char* args[SIZE], int isFore);
void freePointers(char* args[SIZE], char* fileIn, char* fileOut);
void buildInCommands(char* args[SIZE], char* fileIn, char* fileOut, int isFore);
void execute(char* args[SIZE], char* fileIn, char* fileOut, int isFore);

// Declare sigaction function for signal handler.
struct sigaction action;

// A couple of global variables to make life easier ;-)
int status = 0;             // keep current status output.
int pidCounter = 0;         // counts how many processes have been created.

/***********************************************************
* tokenize(): Function to parse and tokenize user's input. *
***********************************************************/
void tokenize(char* input, char* args[SIZE], int isFore) {

    // Set local variables.
    char* token;
    int position = 0;
    char* fileIn = NULL;    // input file name
    char* fileOut = NULL;   // output file name
    
    //tokenize very first argument.
    token = strtok(input, " \n");
    
    while (token != NULL) {
        // Set token as the input file name when "<".
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " \n");
            fileIn = strdup(token);
            token = strtok(NULL, " \n");
            
        // Set token as the output file name when ">".
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " \n");
            fileOut = strdup(token);
            token = strtok(NULL, " \n");
            
        // Set the command as a background process.
        } else if (strcmp(token, "&") == 0) {
            isFore = 0;
            break;
            
        // This token will be the command name or argument.
        } else {
            args[position] = strdup(token);
            token = strtok(NULL, " \n");
            ++position;
        }
    }
    // Set the very last array element to NULL so the
    // program can detect the end of the input string.
    args[position] = NULL;
    
    // Call function to run build-in commands.
    buildInCommands(args, fileIn, fileOut, isFore);
};

/*************************************************************
* killProcess(): Function to kill any other processes or     *
*                jobs that your shell has started before     *
*                it terminates itself, when the EXIT command *
*                is run.                                     *
*************************************************************/
void killProcess() {
    
    // Get pid of first waiting process.
    pid_t pid = waitpid(-1, &status, WNOHANG);
    
    // Kill all pending processes.
    while (pid > 0) {
        if ((kill(pid, SIGKILL)) > 0) {
            //printf("Process %i has been killed!\n", pid);
            pid = waitpid(-1, &status, WNOHANG);
        }
    }
    kill(0, SIGTERM); // double check everything was killed!
};

/**************************************************************
* buildInCommands(): Function to run the three built-in       *
*                    commands: exit, cd, and status. These 3  *
*                    built-in commands are the only ones that *
*                    the shell handles itself. This function  *
*                    also handles blank spaces or comments    *
*                    when they are entered by the user.       *
**************************************************************/
void buildInCommands(char* args[SIZE], char* fileIn, char* fileOut, int isFore) {
    
    int exec; // Dummy variable.
    
    // Check if the input is a blank line or a comment.
    if (args[0] == NULL||*(args[0]) == '#'||*(args[0]) == ' ') {
        // Do nothing. Just reprint cursor and give a
        // a value to the dummy variable.
        exec = 0;
        
    // Check if the user wants to see the exit status or
    // the terminating signal of the last foreground process.
    } else if (strcmp(args[0], "status") == 0) {
        // Print out function.
        printStatus(status);
    
    // Check if the user wants to change directory.
    } else if (strcmp(args[0], "cd") == 0) {
        // Go to home directory if no directory is specified.
        if (args[1] == NULL) {
            chdir(getenv("HOME"));
        // Go to the specified directory.
        } else {
            chdir(args[1]);
        }
    // Check if the user wants to kill all processes or jobs
    // that your shell has started before and exit from the shell.
    } else if (strcmp(args[0], "exit") == 0) {
        // Free active pointers.
        freePointers(args, fileIn, fileOut);
        // Kill all children processes.
        killProcess();
        // Exit the shell.
        exit(0);
    
    // Execute any command other than the build-ins commands.
    } else {
        // Call function to execute non build-in commands.
        execute(args, fileIn, fileOut, isFore);
    }
};

/********************************************************
* printStatus(): Function to print out status outcome   *
*                and termination signals.               *
********************************************************/
void printStatus(int status) {

    // Return a nonzero value if the child process terminated normally.
    if (WIFEXITED(status)) {
        printf("exit value %i\n", WEXITSTATUS(status));
    
    // Otherwise display the termination signal.
    } else {
        printf("terminated by signal %i\n", status);
    }
};

/*********************************************************
* execute(): Function to execute the input commands and  *
*            arguments. It also performs the piping      *
*            process and handles of the input/output     *
*            file redirection.                           *
*********************************************************/
void execute(char* args[SIZE], char* fileIn, char* fileOut, int isFore) {

    int in_descriptor = -1;     // input file descriptor.
    int out_descriptor = -1;    // output file descriptor.
    pid_t pidArray[SIZE];       // store PIDs of the processes created.
    pid_t pid;                  // store current PID.

    // Call fork() to start the parent and child processes
    pid = fork();
    
    // Save most recent pid in the array designated to
    // keep the pid of all processes.
    pidArray[pidCounter] = pid;
    pidCounter++; // Increment pid counter.

    // Child Process: This process does any needed input/output redirection
    // before running exec() on the command given. It uses dup2() to set up
    // the redirection. The redirection symbol and redirection destination
    // and/or source are NOT passed into the following exec command.
    if (pid == 0) {
        // If it's a foreground process, set the signal handler to
        // default so the foreground commands can be interrupted.
        if (isFore == 1) {
            // Set the default action for the signal handler.
            action.sa_handler = SIG_DFL;
            sigaction(SIGINT, &action, NULL);
            
        // If it's a background process, make sure it doesn't send
        // its standard output to the screen.
        } else {
            // Standard input redirected from /dev/null if the user didn't
            // specify some other file to take standard input from.
            in_descriptor = open("/dev/null", O_RDONLY);
            if (in_descriptor == -1) {
                perror("open");  //display error.
                _Exit(1);
            }
            // Replace standard input with "/dev/null" and check for error.
            if (dup2(in_descriptor, 0) == -1) {
                perror("dup2");  //display error.
                _Exit(1);
            }
        }
        //handle input redirection
        if (fileIn != NULL) {
            // open input file and check for error.
            in_descriptor = open(fileIn, O_RDONLY); //set file to be read.
            if (in_descriptor == -1) { //check opening error.
                printf("Error: cannot open %s for input\n", fileIn);
                fflush(stdout);
                _Exit(1);
            }
            // Replace standard input with input file and check for error.
            if (dup2(in_descriptor, 0) == -1) {
                perror("dup2");
                _Exit(1);
            }
            close(in_descriptor);
        //handle output redirection
        } else if (fileOut != NULL) {
            // open output file and check for error.
            out_descriptor = open(fileOut, O_WRONLY | O_CREAT | O_TRUNC, 0744);
            if (out_descriptor == -1) {
                printf("Error: cannot open %s for output\n", fileOut);
                fflush(stdout);
                _Exit(1);
            }
            // Replace standard output with output file and check for error.
            if (dup2(out_descriptor, 1) == -1) {
                perror("dup2");
                _Exit(1);
            }
            close(out_descriptor);
        }
        //execute the command/arguments and check for error.
        if (execvp(args[0], args)) {
            printf("Error: command \"%s\" not found\n", args[0]);
            fflush(stdout);
            _Exit(1);
        }
    // Check if there was an error with fork()
    } else if (pid < 0) {
        perror("fork");
        status = 1;
    
    // Parent Process (shell): It runs continuously. Whenever a
    // non-built in command is received, it forks off a child.
    } else {
        if (isFore == 1) {
            // Wait for completion of foreground commands before
            // prompting for the next command..
            waitpid(pid, &status, 0);
        } else {
            // Otherwise, it's a background process. So print the
            // pid of the background process.
            printf("background pid is %i\n", pid);
        }
    }
    // Call function to free memory designated to pointer variables.
    freePointers(args, fileIn, fileOut);
    // Call function to check for finished background processes.
    finishProcesses(pid);
};

/****************************************************************
* read_input(): Function to read user's input                   *
****************************************************************/
char *read_input(void) {
    char *input = NULL;
    ssize_t bufsize = 0; // have getline allocate a buffer.
    getline(&input, &bufsize, stdin);
    return input; // Return processed input.
};

/*****************************************************************
* freePointers(): Function to free the char pointers, which are  *
*                 going to be reused by the shell in the future. *
*****************************************************************/
void freePointers(char* args[SIZE], char* fileIn, char* fileOut) {

    int i;
    // Free char array created to store arguments or commands.
    for (i = 0; args[i] != NULL; ++i) {
        free(args[i]);
    }
    // Free input and output file-names variables.
    free(fileIn);
    fileIn = NULL;
    free(fileOut);
    fileOut = NULL;
};

/****************************************************************
* finishProcesses(): Function to check for finished background  *
*                    processes and print out their PIDs.        *
****************************************************************/
void finishProcesses(pid_t pid) {
    // Get pid of waiting/zombie processes.
    pid = waitpid(-1, &status, WNOHANG);
    
    // Display the pid of the background processes.
    while (pid > 0) {
        printf("background pid %i is done: ", pid);
        printStatus(status);
        pid = waitpid(-1, &status, WNOHANG);
    }
};

/*************************************************************
* main(): Function to call all functions an run the shell.   *
*************************************************************/
int main(int argc, char **argv) {

    // Set local main variables.
    char* args[SIZE];   // array to store input arguments
    char* input;        // store user input.
    int isFore;         // flag to identify foreground or background process.
    int key = 1;        // Loop key
 
    //set up the signal handler behavior. Code model taken from:
    //https://www.gnu.org/software/libc/manual/html_node/Sigaction-Function-Example.html
    // Make signals can be ignored.
    action.sa_handler = SIG_IGN;
    // Make no flags interrupt the shell execution.
    action.sa_flags = 0;
    // Make no signals interrupt the shell execution.
    sigfillset(&(action.sa_mask));
    sigaction(SIGINT, &action, NULL);
    
    // Main command loop.
    do {
        // The default action is to take all
        // processes as foreground processes.
        isFore = 1;

        // Print constant prompt.
        printf(": ");
        
        // Clean prompt using fflush to eliminate garbage from buffer.
        fflush(stdout);
        
        // Get user input.
        input = read_input();
        
        // Tokenize Input.
        tokenize(input, args, isFore);
        
    } while (key == 1); //Loop conditional.
    
    return 0; //End main.
};





