/* Author: David Eaton
 * Date: 05/07/2020
 * Last Revised: 05/19/2020
 * File Name: smallsh.c
 * Description: This program creates a small shell for a linux environment using C. The shell supports 3 built-in commands: exit, cd, status. These 3 built-in commands are
 * 		handled by the shell itself and do not support manual background/foreground functionality. For non-built-in commands both background and foreground
 * 		functionality is supported. Comments starting with # are supported as well. This program can support command lines with a maximum of 2048 characters and a 
 * 		maximum of 512 arguments(command [arg1 arg2 ...arg512]). There is no support for quoting(arguments with spaces), and no support for the | operator. SIGINT
 * 		terminates foreground commands only, SIGTSTP turns off/on foreground-only mode.
 * Citations:
 * 		1. "Sending a Signal to Another Process: System Call kill()" http://www.csl.mtu.edu/cs4411.ck/www/NOTES/signal/kill.html, Assisted with kill()
 * 		2. Brewster, Benjamin "Signals", Assisted with sending signals to processes.
 * 		3. "What is the best way in C to convert a number to a string?" https://www.geeksforgeeks.org/what-is-the-best-way-in-c-to-convert-a-number-to-a-string/,
 * 		    Assisted with converting a number to a string 
 */

#include <stdio.h>
#include "smallsh_builtins.h"

#define BUFFER_MAX 2048
#define COMMAND_PROMPT_MAX 2
#define ARG_MAX  512 
#define BACKGROUND_MAX 1000

int background_switch = 1;	//Switch for background functionality, 1 means enabled and 0 means disabled. Default is enabled.

/* Signal Handler Function Prototypes */
void backgroundSwitch(int);         //This function enables/disables background functionality

int main() {
	/* Variables needed by the shell */
	int exitFlag = 0;					//Signals the prompt for command loop to exit.
	char commandPrompt[COMMAND_PROMPT_MAX] = ": ";		//Command line prompt
	char* inputBuffer = NULL; 				//Stores user command line input
	char* expandedInput = NULL;				//Stores processed inputBuffer with expanded $$ 
	size_t bufferSize = 0;					//Holds size of allocated buffer from getline
	int numCharsEntered = -1;				//Holds the number returned from getline(the number of characters entered), used for error checking.
	int count = -1;						//Holds the count of initialized elements in arrayOfArgs
	char** arrayOfArgs;					//Holds an array of parsed arguments from the inputBuffer	
	int  i;
	pid_t PID = getpid();					//Stores shell's PIDi
	char sPID[50]; memset(sPID, '\0', sizeof(sPID));	//string version for variable expansion
	sprintf(sPID, "%d", (int)PID);				//Convert to string

	struct process* lastFP= malloc(sizeof(struct process));		//Stores most recent foreground process 
	initProcess(lastFP);	//Initialize lastFP	

	struct process* backgroundPs[BACKGROUND_MAX];		//Stores background processes 
	int bCount = 0;						//count for background processes	

	/* Signal Handler Setup */
	struct sigaction SIGTSTP_action = {0}, SIGINT_action = {0};	//Completely initialize both sigaction structs to be empty
		
	SIGTSTP_action.sa_handler = backgroundSwitch;		//SIGTSTP should disable background functionality of the shell.
	sigfillset(&SIGTSTP_action.sa_mask);			//Block all signals while handler is being executed
	SIGTSTP_action.sa_flags = 0;				//No special flags
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);		//Register signal handler for SIGTSTP

	SIGINT_action.sa_handler = SIG_IGN;			//Smallsh and background processes should ignore SIGINT
	sigaction(SIGINT, &SIGINT_action, NULL);

	/* Create while loop to continously prompt user for commands and arguments. End while loop upon recieving the comand exit */
	do {

		while (count <= 0) {	//If nonblank input is recieved then loop will break, else keep prompting for command

			/* Check background process */
			backgroundChecker(backgroundPs, &bCount);

			/* Prompt user for command via ':' */
			if (write(STDOUT_FILENO, commandPrompt, COMMAND_PROMPT_MAX) != COMMAND_PROMPT_MAX) {
				write(STDERR_FILENO, "Problem writing commandPrompt to standard out\n", 46);	//Write error to stderror if not able to write commandPrompt to stdout 
				return -1;
			}
			fflush(stdout);		//Flush out stdout to force output to screen
		
			/* Get user input */
			numCharsEntered = getline(&inputBuffer, &bufferSize, stdin);	//Read input into inputBuffer from stdin, dynamically create space required.
			if (numCharsEntered == -1) {
				clearerr(stdin);
			} else {
				if (inputBuffer[0] != '#') {		//Make sure input is not a comment	
					/* Check and expand all cases of $$ */
					expandedInput = variableExpansion(inputBuffer, sPID);
					
					/* Parse input from user */
					arrayOfArgs = parseBuffer(expandedInput, &count, BUFFER_MAX, ARG_MAX, background_switch);

				/*	// Uncomment to see parsed arguments 
					printf("\n\n");
					fflush(stdout);
					for(i = 0; i < count; i++) {
						printf("Argument %d: %s\n", i, arrayOfArgs[i]);
						fflush(stdout);
					}

				*/	

					/* Validate that the Prompt is a max of 2048 characters and a max of 512 arguments */
					if (count > ARG_MAX || strlen(inputBuffer) > BUFFER_MAX) { 
						count = 0;	//Reset count to zero to repromt for a command 
						fprintf(stderr, "Error, commands should be a max of %d characters and a max of %d arguments\n", BUFFER_MAX, ARG_MAX); 
					}
					
				}

					free(inputBuffer);	//Free inputBuffer to avoid memory leaks
					inputBuffer = NULL;
					free(expandedInput);	//Free expandedInput
					expandedInput = NULL;
				
			}
		}		
	
		/* Command Handler via switch statement */
		exitFlag = commandHandler(arrayOfArgs, &count, lastFP, backgroundPs, &bCount);

		/* Free arrayOfArgs */
		freeArgs(arrayOfArgs, ARG_MAX);
		count = 0;	
	} while(exitFlag == 0);

	/* Free backgroundPs to avoid memory leaks */
	for (i = 0; i < bCount; i++) {
		free(backgroundPs[i]);
		backgroundPs[i] = NULL;
	}

	free(lastFP);	//Free lastFP
	lastFP = NULL;
	return 0;
}


/*				          backgroundSwitch
 * Description: This function enables/disables background functionality of the smallsh script by updating the global flag "background_switch".
 * Parameters: int signo
 * Returns: void
 * Preconditions: N/A 
 */
void backgroundSwitch(int signo) {
    
    /* If switch is in on position turn it to off position and display message */
    if (background_switch == 1) {
        background_switch = 0;    //Turn off background functionality
        write(STDOUT_FILENO, "Entering foreground-only mode (& is now ignored)\n", 49);
        fflush(stdout);     //Flush buffer to screen
    }
    
    else if (background_switch == 0) {      //If in off position turn switch on and display message
        background_switch = 1;    //Turn on background functionality
        write(STDOUT_FILENO, "Exiting foreground-only mode\n", 29);
        fflush(stdout);
        
    }
   
}
