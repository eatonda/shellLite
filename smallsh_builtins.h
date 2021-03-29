/* Author: David Eaton
* Date: 05/07/2020
* Last Revised: 05/19/2020
* File Name: smallsh_builtins.h
* Description: This is the function declaration file for the built in functions utilized in the smallsh shell. This
*              includes functions that assist the shell with function executions.
* Citations:
*         1. Chauhan, Anuj "C program to Replace a word in a text by another given word"
              https://www.geeksforgeeks.org/c-program-replace-word-text-another-given-word/
              Assisted with overall design of variableExpansion function.
          2. Brewster, Benjamin "Processes" Assisted with knowledge of getting process ids, using fork(), execvp(), and overall design of setting up multiple processes within a program.
          3. Brewster, Benjamin "Process Management & Zombies" Assisted with design for process management.
          4. Brewster, Benjamin "Signals" Assisted with setting up signal handlers
          5. Brewster, Benjamin "More UNIX I/O" Assisted with I/O redirection
*/
#ifndef smallsh_builtins_h
#define smallsh_builtins_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <fcntl.h>

                            /* Struct to make storing info about processes easier */
struct process {
    pid_t pid;            //Stores process id
    int exitValue;        //Stores exit value
    int signalValue;        //Stores signal value
};

void initProcess(struct process* p);    //Function to initialize process


                                    /* Built in Functions */
char** parseBuffer(char*, int*, int, int, int);    //Parses commands given by user and returns an array of string commands.
    
int linearSearch(char**, char*, int);        //Returns index of word from char** if found, or returns -1
    
void freeArgs(char** , int);            //Frees the char** array holding parsed arguments, implemented for readability and to prevent memory leaks.
    
int commandHandler(char**, int*, struct process*, struct process* [], int*);    //Routes commands for execution
    
void status(struct process*);            //Displays either the exit value or signal value of the most recent foreground process
    
void exitShell(struct process* [], int count);    //Terminates active background processes
    
void cd(char [], int);        //Changes working directory of shell
    
void commandLauncher(char**, int*, struct process*, struct process* [], int*);    //Launches nonbuilt-in commands
    
void backgroundChecker(struct process* [], int*);    //Checks and cleans up completed background processes
    
char* variableExpansion(char*, char *);        //This function replaces all instances of $$ with the pid of the shell
    
void shiftLeft(char**, int s, int c);            //Shifts array of strings to left by 1 element, starting at int s and ending at end c




#endif /* smallsh_builtins_h */
