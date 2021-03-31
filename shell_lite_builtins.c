/* Author: David Eaton
* Date: 05/07/2020
* Last Revised: 05/19/2020
* File Name: shell_lite_builtins.c
* Description: This is the function implementation file for the built in functions utilized in the shellLite shell.               This includes functions that assist the shell with function executions.
* Citations:
*         1. Chauhan, Anuj "C program to Replace a word in a text by another given word"
              https://www.geeksforgeeks.org/c-program-replace-word-text-another-given-word/
              Assisted with overall design of variableExpansion function.
          2. Brewster, Benjamin "Processes" Assisted with knowledge of getting process ids, using fork(), execvp(), and overall design of setting up multiple processes within a program.
          3. Brewster, Benjamin "Process Management & Zombies" Assisted with design for process management.
          4. Brewster, Benjamin "Signals" Assisted with setting up signal handlers
          5. Brewster, Benjamin "More UNIX I/O" Assisted with I/O redirection
*/
#include "smallsh_builtins.h"

/* Function to initialize process */
void initProcess(struct process* p) {
    assert(p != NULL);
    p->pid = 0;
    p->exitValue = -1;
    p->signalValue = -1;
}

/*            parseBuffer
 * Description: This function parses the buffer argument by space and returns an array of strings where each element is a command/argument. Upon successful completion
 *         the function updates the count argument by reference with the number of initialized elements.
 * Parameters: char buffer[], int* count, int maxChars, int maxSize, int flag (flag for background functionality)
 * Returns: char** updates count by reference.
 * Preconditions: maxChar > 0 maxSize > 0
 */
char** parseBuffer(char* buffer, int* count, int maxChars, int maxSize, int flag) {
    assert(maxChars > 0 && maxSize > 0);        //Assert preconditions have been met.

    int i;                //index to iterate through loops
    char* word;            //Stores parsed string from strtok call
    char test[maxChars];        //Used to test input against a blank command
    memset(test, '\0', maxChars);
 
    /* Set up array of strings to hold parsed commands/arguments */
    char** arrayOfArgs = malloc(maxSize * sizeof(char*));        //Dynamically create memory to hold maxSize strings
    assert(arrayOfArgs != NULL);            //Assert the dynamic allocation was successful
    for (i = 0; i < maxSize; i++) {
        arrayOfArgs[i] = NULL;            //Initialize all to NULL
    }

    /* Parse buffer argument */
    i = 0;
    word = strtok(buffer, " ");
    while (word != NULL) {
        word[strcspn(word, "\n")] = '\0';    //Remove newline character getline adds
        
        /* Dynamically create space in arrayOfArgs to hold parsed word from buffer */
        /*
        arrayOfArgs[i] = (char*)malloc(sizeof(char*) * strlen(word));
        assert(arrayOfArgs[i] != NULL);
        memset(arrayOfArgs[i], '\0', sizeof(arrayOfArgs[i]));
         */
        arrayOfArgs[i] = (char*)malloc((strlen(word)+ 1));
        assert(arrayOfArgs[i] != NULL);
        memset(arrayOfArgs[i], '\0', (strlen(word)+ 1));
 
        strcpy(arrayOfArgs[i], word);    //Assign parsed word to arrayOfArgs
        i++;             //increment index
        word = strtok(NULL, " ");    //Get next word
    }

    if (strcmp(arrayOfArgs[0], test) == 0) { i = 0; }    //Empty command recieved
     
    *count = i;                    //Update count to be the number of initialized elements in arrayOfArgs

    /* Check flag to see if background functionality is enabled */
    if (flag == 0) {
        if (strcmp(arrayOfArgs[*count - 1], "&") == 0) {
            /* if disabled remove background operator if there is one and update count */
            free(arrayOfArgs[*count - 1]); arrayOfArgs[*count - 1] = NULL; //Free memory make pointer safe
            *count = (*count) - 1;  //Decrement count
        }
        
    }
    
    return arrayOfArgs;                //Returned parsed arguments
}

/*            linearSearch
 * Description: This function performs a linear search of the char** array. Returns the index if found or -1 if not.
 * Parameters: char** args, char* word, int count
 * Returns: index if found -1 if not
 * Preconditions: char** args != NULL, word != NULL, count >= 0
 */
int linearSearch(char** args, char* word, int count) {
    assert(args != NULL && word != NULL && count >= 0);

    int i;
    for (i = 0; i < count; i++) {
        if (strcmp(args[i], word) == 0)
            return i;        //Return found index
    }

    return -1;
}
                      
/*            freeArgs
 * Description: This function frees all allocated memory associated with arrayOfArgs.
 * Parameters: char** array, int size
 * Returns: NA
 * Preconditions: char** array != NULL, int size > 0
 */
void freeArgs(char** array, int size) {
    assert(array != NULL && size > 0);

    int i;
    for ( i = 0; i < size; i++) {
        free(array[i]);        //Free element in array of strings
        array[i] = NULL;    //Make pointer safe
    }

    free(array);    //Free memory of the array itself
    array = NULL;
}

/*            commandHandler
 * Description: This function routes commands to their appropriate if/else blocks and executes("handles them"). This function assists with overall readabilty of main.
 * Parameter: char** args, int* aCount(count of args), struct process* lastForeground, struct process* backgroundPs[], int* bCount(count of backgroundPs)
 * Returns: 1 if command exit executes else returns 0
 * Preconditions: args != NULL, aCount > 0, bCount >= 0
 */
int commandHandler(char** args, int* aCount, struct process* lastForeground, struct process* backgroundPs[], int* bCount) {
    assert(args != NULL && aCount > 0);
    assert(*bCount >= 0);

    /* Utilize the first element in args(the command) to route command to proper execution point */
    if (strcmp(args[0], "cd") == 0) {
        cd(args[1], *aCount);    //Change directories

    } else if (strcmp(args[0], "exit") == 0) {
        exitShell(backgroundPs, *bCount);    //Kill background running processes
        return 1;    //Signal smallsh to terminate

    } else if (strcmp(args[0], "status") == 0) {
        status(lastForeground);    //Get status of most recent foreground process

    } else {
        commandLauncher(args, aCount, lastForeground, backgroundPs, bCount);     //Launch non-builtin commands
    }
    return 0;
}


/*            cd
 * Description: This function changes the working directory of the smallsh shell. With no arguments it changes to the HOME directory and with 1 argument it changes
 *         to the path argument specified. Supports both absolute and relative paths.
 * Parameters: char path[], int argCount
 * Returns: void
 * Preconditions: argCount >= 1
 */
void cd(char path[], int argCount) {
    assert(argCount >= 1);

    if (argCount == 1) {        //cd with 0 arguments
        if (chdir(getenv("HOME"))) {
            printf("Problem navigating to home environment variable directory\n");    //If there is a problem navigating to the home directory display error
            fflush(stdout);
        }
    } else if (argCount == 2) {    //cd with 1 argument
            if (chdir(path)) {
            printf("Problem couldn't change to %s\n", path);    //If there is a problem changing to absolute path
                fflush(stdout);
            }
    }
}

/*            exitShell
 * Description: This function exits the smallsh shell. It causes the smallsh to kill any processes or jobs that the smallsh shell has started before it terminates
 *         itself.
 * Parameters: struct process* backgroundPs, int count
 * Returns: void
 * Preconditiones: N/A
 */
void exitShell(struct process* backgroundPs[], int count) {
    pid_t completedPID = -1;    //Stores PID of completed process from backgroundPIDs
    int i;
    int exitMethod = -5;        //Stores exit method from waitPID

    if (count > 0) {
        /* Kill any running background process */
        for (i = 0; i < count; i++) {
            if (completedPID == 0) {
                /* If running kill process */
                kill(backgroundPs[i]->pid, SIGTERM);
            }
        }
    }
}

/*            status
 * Description: This function prints out either the exit status or terminating signal of the last foreground process ran by smallsh shell.
 * Parameters: struct process* last
 * Returns: void
 * Preconditions:  last != NULL
 */
void status(struct process* last) {
    assert( last != NULL);

    /* No foreground process has occured */
    if (last->pid <= 0) {
        printf("exit value %d\n", 0);    //Display exit value to terminal
        fflush(stdout);
    } else {
        /* EXIT case */
        if (last->exitValue >= 0) {
            printf("exit value %d\n", last->exitValue);    //Display exit value to terminal
            fflush(stdout);
        }
         
        /* SIGNAL Case */
        if (last->signalValue >= 0) {
            printf("terminated by signal %d\n", last->signalValue);    //Display the signal to the terminal
            fflush(stdout);
        }

    }
}


/*            commandLauncher
 * Description: This function launches nonbuilt-in commands by fork and exec. lastForegroundPID as well as bCount and backgroundPIDs are updated
 *         via reference.
 * Parameters: char** args, int* aCount, struct process* lastForeground, struct process* backgroundPs[], int* bCount
 * Returned: void
 * Preconditions: args != NULL, aCount >=1, lastForeground != NULL, backgroundPs != NULL, bCount != NULL
 */
void commandLauncher(char** args, int* aCount, struct process* lastForeground, struct process* backgroundPs[], int* bCount) {
    assert(args != NULL && *aCount >= 1);
    assert(lastForeground != NULL && backgroundPs != NULL && bCount != NULL);

    int i, j;        //index
    pid_t spawnPID = -5;        //Stores PID from fork()
    int childExitStatus = -5;    //Stores exitStatus of child process
    int fd = -5;            //file descriptor

    /* Create a child process to carry out command execution */
    spawnPID = fork();
    switch (spawnPID) {
        case -1: {perror("Error with fork in commandLauncher\n"); exit(1); break;}    //Error case

        case 0: {
                /* Check for redirection */
                int stdinRedirectionOperator = linearSearch(args, "<", *aCount);    //index of ">" in args

                if ( stdinRedirectionOperator > -1) {
                    fd = open(args[stdinRedirectionOperator + 1], O_RDONLY);  //Open file for reading only
                    if (fd == -1 ) { perror("open() error\n"); exit(1); }     //Print error if issue and set exit value to 1
                        
                    if ((dup2(fd, 0)) == -1) {     //Redirect standard input to target given.
                        perror("dup2 stdin redirection fail!\n");        //Print error if unsuccessful
                        exit(1);
                    }

                    /* Get rid of background operator for execvp function */
                    free(args[stdinRedirectionOperator]);  args[stdinRedirectionOperator] = NULL;
                    shiftLeft(args, stdinRedirectionOperator, *aCount);    //Close gap in args
                    *aCount = (*aCount - 1);    //Decrement args count by reference

                    /* Get rid of file path from args */
                    free(args[stdinRedirectionOperator]); args[stdinRedirectionOperator] = NULL;    //Note previous shift
                    shiftLeft(args, stdinRedirectionOperator, *aCount);
                    *aCount = (*aCount - 1);
    
                }

                int stdoutRedirectionOperator = linearSearch(args, ">", *aCount);       //index of "<" in args
                if (stdoutRedirectionOperator > -1) {
                    fd = open(args[stdoutRedirectionOperator + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);  //Open file for writing only, create or Trunc
                    if (fd == -1 ) { perror("open() error\n"); exit(1); }     //Print error if issue and set exit value to 1
                        
                    if ((dup2(fd, 1)) == -1) {     //Redirect standard output to target  given.
                        perror("dup2 stdout redirection fail!\n");        //Print error if unsuccessful
                        exit(1);
                    }

                    /* Get rid of background operator for execvp function */
                    free(args[stdoutRedirectionOperator]);  args[stdoutRedirectionOperator] = NULL;
                    shiftLeft(args, stdoutRedirectionOperator, *aCount);
                    *aCount = (*aCount - 1);
 
                    /* Get rid of file path from args */
                    free(args[stdoutRedirectionOperator]);  args[stdoutRedirectionOperator] = NULL;
                                        shiftLeft(args, stdoutRedirectionOperator, *aCount);
                                        *aCount = (*aCount - 1);

                }
                
                /* Background with no redirection targets given */
                if (strcmp(args[*aCount - 1], "&") == 0 && stdoutRedirectionOperator == -1 && stdinRedirectionOperator == -1) {
                    /* Output redirection */
                    fd = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);    //Open file for writing only, create one or truncate
                    if (fd == -1 ) { perror("open() error\n"); exit(1); }     //Print error if issue and set exit value to 1
                        
                    if ((dup2(fd, 1)) == -1) {     //Redirect background standard output to /dev/null if no target is given.
                        perror("dup2 background default stdout redirection fail!\n");        //Print error if unsuccessful
                        exit(1);
                    }

                    /* Input redirection */
                    fd = open("/dev/null", O_RDONLY);    //Open file for read only
                    if (fd == -1 ) { perror("open() error\n"); exit(1); }        //Error handling for open()

                    if ((dup2(fd, 0)) == -1) {    //Redirect background standard input to be from /dev/null if no target is given
                        perror("dup2 background default stdin redirection fail!\n");
                        exit(1);
                    }

                }

                /* Get rid of background operator for exexvp function */
                if (strcmp(args[*aCount - 1], "&") == 0) {
                    free(args[*aCount - 1]);  args[*aCount - 1] = NULL;
                    *aCount = (*aCount - 1);    //Decrement args count by reference
                    
                }
                else {
                    /* Set up child foreground process to respond to SIGINT */
                    struct sigaction SIGINT_action = {0};
                    SIGINT_action.sa_handler = SIG_DFL;     //Set foreground processes to respons normally to SIGINT
                    sigaction(SIGINT, &SIGINT_action, NULL);
                }
            
                /* Execute Command */
                if (execvp(*args, args) < 0) {
                    perror("Exec failure!\n");        //If there is a problem with executing command print error and set exit status to  1
                    exit(1);
                }
                break;
            }

        default: {
                /* Check to see if command should be executed in the background or the foreground */
                if (strcmp(args[*aCount - 1], "&") == 0) {
                    printf("Background pid is %d!\n", spawnPID);
                    fflush(stdout);

                    /* Add background process info to backgroundPs */
                    backgroundPs[*bCount] = malloc(sizeof(struct process));    //Dynamically allocate memory for new process struct
                    initProcess(backgroundPs[*bCount]);    //Initialize element
                    backgroundPs[*bCount]->pid = spawnPID;    //Save pid
                    *bCount = (*bCount + 1);    //Increment count of background processes
                } else {
                    initProcess(lastForeground);        //reset lastForeground;s variables for subsequent foreground processess.
                    
                    lastForeground->pid = waitpid(spawnPID, &childExitStatus,0);        //Ensure foreground process completes and save pid

                    /* EXIT case */
                    if (WIFEXITED(childExitStatus)) {
                        lastForeground->exitValue = WEXITSTATUS(childExitStatus);    //Store exit status value
                    }
         
                    /* SIGNAL Case */
                    if (WIFSIGNALED(childExitStatus)) {
                        lastForeground->signalValue = WTERMSIG(childExitStatus);    //Store Signal Value
                        status(lastForeground);   //Print signal that terminated foreground process
                    }

                }
                break;
            }
    }
}


/*            backgroundChecker
 * Description: This function checks for completed background processes via waitpid. This function utilizes the NOHANG flag so that commands can still continue
 *         in the foreground. This process also cleans up background process as they finish and displays the process that has finished.
 * Parameters: struct process* arr, int* count
 * Returns: void
 * Preconditions: arr != NULL, *count >= 0
 */
void backgroundChecker(struct process* arr[], int* count) {
    assert(arr != NULL && *count >= 0);

    int completedFlag = -1;
    int exitStatus = -5;
    int i, j;
 
    for ( i = 0; i < *count; i++) {
        completedFlag = waitpid(arr[i]->pid, &exitStatus, WNOHANG);    //See if process is complete

        if (completedFlag > 0) {
            printf("background pid %d is done: ", completedFlag);    //Display message stating process is complete
            fflush(stdout);

            /* EXIT case */
            if (WIFEXITED(exitStatus)) {
                printf("exit value %d\n", WEXITSTATUS(exitStatus));    //Display exit status of process
                fflush(stdout);
            }
         
            /* SIGNAL Case */
            if (WIFSIGNALED(exitStatus)) {
                printf("terminated by signal %d\n", WTERMSIG(exitStatus));    //Display signal value
                fflush(stdout);
            }

            /* Free memory and close gap in array */
            free(arr[i]);
            arr[i] = NULL;    //Make pointer safe
            for ( j = i; j < *count; j++) {
                arr[j] = arr[j + 1];    //Close gap in arr
            }
            *count = (*count - 1);    //Update count of arr

        }
        completedFlag = -1;    //reset flag
    }
}

/*            variableExpansion
 * Description: This function replaces all instances of $$ with the shell process id. This function returns a new dynamically allocated string.
 * Parameters: char* original, char pid[]
 * Returns: New string with $$ expanded to pid
 * Preconditions: No parameters can be NULL
 */
char* variableExpansion(char* original, char* pid) {
    assert(original != NULL &&  pid != NULL);
   // printf("pid: %s\n", pid);

    int varLength = 2;    //Length of $$
    int pidLength = strlen(pid);    //Length of pid


    /* Get frequency of "$$" in orgianl for proper memory allocation of product */
    int i;    //index
    int freq = 0;
    int subCount = 0;   //1 freq = two subCount
    for (i = 0; i < strlen(original); i++) {
        if (original[i] == '$') {
            subCount++;     //Count character
        } else {
            subCount = 0;   //If not '$' reset subCount
        }
        
        if (subCount == 2) {
             freq++;        //Count each occurance of "$$"
            subCount = 0;   //Reset subCount
        }
    }

    //Uncomment below to see freq for troubleshooting
    //printf("frequency %d\n", freq);
    int productSize = (strlen(original) + (freq * (pidLength - varLength))) + 1;
    
    char* product = malloc(productSize);    //stores new string with replacements made
    assert(product != NULL);
    memset(product, '\0', productSize);        //Initialize with null terminators for safety


    /* Make replacements */
    subCount = 0;
    int j = 0;
    for (i = 0; i < strlen(original); i++) {
        if (original[i] == '$') {
            if (subCount == 0 && i + 1 < strlen(original) && original[i+1] != '$') {
                 product[j] = original[i];   //Copy over single $ characters
                 j++;
            }
            subCount++;     //Count character
        } else {
            subCount = 0;   //If not '$' reset subCount
            product[j] = original[i];   //Copy over non $ characters
            j++;
        }
        
        if (subCount == 2) {
             freq--;        //Decrement freq for error checking after loop
            
            /* Copy pid into product */
            strcat(product, pid);
            j += pidLength;
            
            subCount = 0;   //Reset subCount
        }
    }
    assert(freq == 0);  //Check to make sure correct number of swaps were made
    
    //Uncomment to check the content of product 
    //printf("%s", product); fflush(stdout);
  

    return product;
}

/*            shiftLeft
 * Description: Shifts contents of arr over 1 element to left starting from s and ending at c
 * Parameters: char** arr, int s, int c
 * Returns: void
 * Preconditions: arr != NULL, s >= 0, c >= 0
 */
void shiftLeft(char** arr, int s, int c) {
    assert(arr != NULL && s >= 0 && c >= 0);

    int i;
    for (i = s; i < c; i++) {
        arr[i] = arr[i+1];
    }
}



