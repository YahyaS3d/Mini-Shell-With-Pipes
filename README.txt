Mini Shell with pipes & signals 
Authored by Yahya Saad

==Description==
This program is a mini shell implementation. The program accepts commands from the user and executes them by calling system calls to the operating system. The program has a simple command prompt that displays the current directory and the number of commands entered, and their total number of arguments.
advanced features also - redirection of output, running commands in the background, and handling pipes.


==Functionality==
The program executes the following basic functions:

Prompting the user for input
Parsing the input commands into individual words
Removing quotes from the commands
Executing the commands using system calls to the operating system
Background allows running a command in the background, catches and handles signals, and redirects command output to a specified file.
Printing error messages for unsupported commands
Exiting the program when three consecutive empty lines are entered.
==Functions== 
DisplayPrompt(): displaying the prompt to the user, which includes the current directory, the number of commands executed, and the number of arguments.
removeQuotes(): This function is responsible for removing double quotes from a string.
ParsingCommand(): parsing a command string into individual words, including the command name and its arguments.
RunInBackground()- allows running a command in the background using the & symbol. This function creates a new process that runs the command in the background.
handle_signal()-  function is a signal handler that is used to catch and handle signals. The shell handles SIGTSTP signals to suspend and resume execution of a command and ignores SIGINT signals.
RedirectOutput()- function creates a child process and redirects the output of the command to a specified file.
RunCommand(): This function is responsible for executing a command if it exists in the operating system. If the command is cd, then it prints an error message because it's not supported. The function takes a pointer to an array of strings, where the first element is the command name, and the rest are its arguments.
main(): execute the program and run it functionality

==Program Files==
ex2.c - this file contains the main program with all functions 
README.txt - description for the program 

==How to compile?==
compile: gcc ex2.c -o ex2
run: ./ex2

==Input:==
OS commands from the user, which supported in bash\shill 

==Limitations==
The program does not support the cd command. It also has a limit of 510 characters for each command and a maximum of 10 arguments per command. If either of these limits is exceeded, the program will print an error message and exit.
the user can exit the program if 3 empty lines were entered 

==Output:==
command executed successfully! with a new prompt line which contins the number of args and number of commands that entered until now :) 



