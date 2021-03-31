
Files: shell_lite.c shell_lite_builtins.c shell_lite_builtins.h makefile
 
Directions for compiling shell_lite .

To compile the program simply type "make" into the command line and press enter. (Don't type "")

Executable name is "shellLite", simply type shell_lite to run.


To remove executables and object files simply type "make clean" and press enter. (Don't type "")


Program Specifications:
 I) The Prompt
  1. : is the symbol of prompt for each command line.
  2. The general syntax of the command line is: command [arg1 arg2 ...] [< input_file] [> output_file] [&]. bracket items are optional.
  3. Commands are made up of words seperated by spaces.
  4. Special symbols: <, >, and & are recognized as in bash shells, note space is required.
  5. Quoting and | operation are not supported.
  6. Any line that begins with # character is treated as a comment line.
  7. // not supported.
  
 II) Command Execution
  1. If command is invalid, value returned is 1.
  2. Shell uses PATH variables to look for non-built in commands.
  3. stdin and stdout can be redirected at the same time.
  
 III) Background and Foreground
  1. Both background and foreground commands are supported, like with bash shells.
  2. When a background process terminates, a message showing the process id and exit status will be printed.
  
 IV) Signals
  1. A CTRL-C command from the keyboard will send a SIGINT signal to parent shell process and all children at the same time, for the exception of the shellLite shell and background processes.
  2. A CTRL-Z command from the keyboard will send a SIGTSTP signal to shell process and all children at the same time. When this signal is received by shellLite shell, a informative message is displayed immediately if it's sitting at the prompt, or immediately after any currently running foreground process has terminated, and then enter a state where subsequent commands can no longer be run in the background. In this state, the & operator is simply be ignored - run all such commands as if they were foreground processes. If the user sends SIGTSTP again, another informative message is displayed immediately after any currently running foreground process terminates, and then return back to the normal condition where the & operator is once again honored for subsequent commands, allowing them to be placed in the background. See the example below for usage and the exact syntax which you must use for these two informative messages. Your foreground and background child processes should all ignore a SIGTSTP signal: only your shell should react to it.
  
  
 V) Built-in Commands
   1. 3 built in commands supported (exit, cd, status). 
   The exit command exits your shell. It takes no arguments. When this command is run, your shell must kill any other processes or jobs that your shell has started before it terminates itself.

    The cd command changes the working directory of your shell. By itself - with no arguments - it changes to the directory specified in the HOME environment variable (not to the location where shellLite was executed from, unless your shell executable is located in the HOME directory, in which case these are the same). This command can also take one argument: the path of a directory to change to. Your cd command should support both absolute and relative paths. When shellLite terminates, the original shell it was launched from will still be in its original working directory. Your shell's working directory begins in whatever directory your shell's executible was launched from.

    The status command prints out either the exit status or the terminating signal of the last foreground process (not both, processes killed by signals do not have exit statuses!) ran by your shell. If this command is run before any foreground command is run, then it should simply return the exit status 0. These three built-in shell commands do not count as foreground processes for the purposes of this built-in command - i.e., status should ignore built-in commands.
    
    
 VI) Example
 <img width="547" alt="Screen Shot 2021-03-31 at 5 12 40 PM" src="https://user-images.githubusercontent.com/59621751/113217964-5798df80-9244-11eb-85e6-4a4599585f92.png">

   
   
  


