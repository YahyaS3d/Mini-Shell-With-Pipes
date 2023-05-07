//ex2.c mini shell with pipes
//Name: Yahya Saad 
//-------------------------------some include for usage------------------------------
#include <stdlib.h>

#include <stdio.h>

#include <unistd.h>

#include <sys/wait.h>

#include <pwd.h>

#include <string.h>

#include <fcntl.h>
//-------------------------------some methods and globals parameters\macros------------------------------
pid_t pid;
#define MAX 510
#define MAX_ARGS 10
//this method is for the prompt line
void DisplayPrompt(int command_counter, int args_count);
//remove double quotes
void removeQuotes(char * str);
//tokenizes the command string into individual words
int ParsingCommand(char * command, char ** countW, int * index);
//execute the command if it's already exist in the operating system
void RunCommand(char ** countW);
//allows running a command in the background using the & symbol
void RunInBackground(char **args);
// signal handler function
void handle_signal(int sig);
//remove the spaces around the pipe | symbol
//void RemoveSpacesFromPipeCommand(char* str);
//creates a child process and redirects the output to the specified file
void RedirectOutput(char **args);
//-------------------------------main program------------------------------
int main() {
    //command counter to print it later
    //commands length for testing usage only
    int command_counter = 0;
    unsigned long commands_length = 0;
    char command[MAX];
    char ** commandArr; //main array to store the parsed output
    int empty_lines = 0; // number of consecutive empty lines
    int totArgs = 0;
    // set up signal handlers
    signal(SIGTSTP, handle_signal);
    signal(SIGINT, SIG_IGN); // ignore SIGINT (Ctrl+C)
    while (1) {
        signal(SIGTSTP, handle_signal); // install signal handler before executing child process
        int countArgs = 0; // reset the count of arguments for each command
        DisplayPrompt(command_counter, totArgs); // print the prompt line with the number of arguments
        fgets(command, MAX, stdin); // receives a command

        if (strcmp(command, "\n") == 0 != 0) { // if empty line is entered
            empty_lines++;
            if (empty_lines == 3) { // if three consecutive empty lines are entered
                printf("Exiting the program!!");
                exit(EXIT_FAILURE);
            }

            continue;
        }

        empty_lines = 0; // reset consecutive empty lines count

        //        command_counter++;
        command[strlen(command) - 1] = '\0'; // delete the \n
        commands_length += strlen(command); // add the length of the current command to the sum

        char * command_list[MAX_ARGS];
        int num_commands = 0;

        char * command_token = strtok(command, ";");
        while (command_token != NULL) {
            command_list[num_commands] = command_token;
            num_commands++;
            command_token = strtok(NULL, ";");
        }
        command_counter += num_commands;

        for (int j = 0; j < num_commands; j++) {
            commandArr = (char ** ) malloc(sizeof(char * ) * MAX); // allocate memory for the array who hold the command words
            int index = 0;

            ParsingCommand(command_list[j], commandArr, & index); // use a method to split the command into a double array
            //now commandArr holds all the arguments received

            if (strcmp(commandArr[0], "cd") == 0) {
                printf("command not supported\n");
            } else if (strcmp(commandArr[0], "bg") == 0) { // handle bg command
                RunInBackground(commandArr);
                countArgs += index;
            } else { //if it needs to run the command
                RunCommand(commandArr);
                countArgs += index; // add the number of arguments to the total count of arguments

            }

            for (int i = 0; i < index; i++) // free the arrays of the words
                free(commandArr[i]);
            free(commandArr);
        }

        totArgs += countArgs; // add the total number of arguments for the current line to the overall count

    }
    return 0;
}
//-------------------------------Helpful functions------------------------------
int ParsingCommand(char* command, char** countW, int* index) {
    char* argument = strtok(command, " ");
    unsigned long arg_size;
    while (argument != NULL) {
        if (*index >= MAX_ARGS) {
            printf("Too many arguments.\n");
            exit(EXIT_FAILURE);
        }
        arg_size = strlen(argument);
        countW[*index] = (char*) malloc(sizeof(char) * (arg_size + 1));
        if (countW[*index] == NULL) {
            printf("An array for part of the command didn't allocate");
            exit(EXIT_FAILURE);
        }
        removeQuotes(argument); // remove double quotes from the argument
//        RemoveSpacesFromPipeCommand(argument); // remove spaces around the pipe symbol
        strcpy(countW[*index], argument);
        (*index)++;
        argument = strtok(NULL, " ");
    }
    countW[*index] = NULL;
    free(argument);
    return *index;
}

void DisplayPrompt(int command_counter, int args_count) {
    struct passwd * pw; // will hold the username
    char * cwd; // will hold the path to current directory
    pw = getpwuid(getuid());
    cwd = getcwd(NULL, 0); // get the path
    //show the path
    printf("#cmd:%d|#args:%d@%s%s> ", command_counter, args_count, pw -> pw_name, cwd); //print the open line
    free(cwd);
}
void removeQuotes(char * str) {
    char * src, * dst;
    for (src = dst = str;* src != '\0'; src++) {
        if ( * src != '"') {
            * dst++ = * src; //remove the quotes
        }
    }
    * dst = '\0';
}

void RunCommand(char **countW) {
    int num_pipes = 0;
    int index = 0;
    while (countW[index] != NULL) {
        if (strcmp(countW[index], "|") == 0) {
            num_pipes++;
        }
        index++;
    }

    int pipe_fds[2 * num_pipes];
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipe_fds + 2 * i) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    int curr_pipe = 0;
    int curr_command = 0;
    int curr_command_start = 0;
    int status;

    while (countW[curr_command] != NULL) {
        if (strcmp(countW[curr_command], "&") == 0) {
            countW[curr_command] = NULL;
            RunInBackground(countW + curr_command_start);
            curr_command_start = curr_command + 1;
        }
        else if (strcmp(countW[curr_command], "|") == 0) {
            countW[curr_command] = NULL;

            if (pipe(pipe_fds + 2 * curr_pipe) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            pid = fork();

            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) { // child process
                if (dup2(pipe_fds[2 * curr_pipe + 1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }

                for (int i = 0; i < 2 * num_pipes; i++) {
                    if (close(pipe_fds[i]) == -1) {
                        perror("close");
                        exit(EXIT_FAILURE);
                    }
                }

                if (execvp(countW[curr_command_start], countW + curr_command_start) == -1) {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            } else { // parent process
                curr_command_start = curr_command + 1;
                curr_pipe++;
            }
        }

        curr_command++;
    }
/*
 *  handling multiple commands separated by pipe '|' symbol.
 *  Each command needs its own child process to execute it
 *  and each child process needs its own pipe to communicate with the previous and next child processes.
 *  Therefore, the code needs to fork multiple times to create multiple child processes to execute multiple commands
 *  and the communication between them requires multiple pipes.
 */
    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // child process
        if (num_pipes > 0) {
            if (dup2(pipe_fds[2 * (num_pipes - 1)], STDIN_FILENO) == -1) {
                perror("dup2");
                exit(EXIT_FAILURE);
            }
        }
        else if (countW[1] != NULL && strcmp(countW[1], ">") == 0) { // handle output redirection
            RedirectOutput(countW);
            exit(EXIT_SUCCESS);
        }

        for (int i = 0; i < 2 * num_pipes; i++) {
            if (close(pipe_fds[i]) == -1) {
                perror("close");
                exit(EXIT_FAILURE);
            }
        }

        if (execvp(countW[curr_command_start], countW + curr_command_start) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else { // parent process
        for (int i = 0; i < 2 * num_pipes; i++) {
            if (close(pipe_fds[i]) == -1) {
                perror("close");
                exit(EXIT_FAILURE);
            }
        }

        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
    }
}

//void RemoveSpacesFromPipeCommand(char* str) {
//    char* src = str;
//    char* dst = str;
//    while (*src) {
//        if (*src == '|') {
//            *dst++ = '|';
//            if (*(src+1) == ' ') {
//                src++;
//            }
//        }
//        *dst++ = *src++;
//    }
//    *dst = 0;
//}
void RunInBackground(char **args) {
    pid_t pxd = fork();
    if (pxd == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pxd == 0) { // child process
        setpgid(0, 0); // put the child process in a new process group
        if (execvp(args[0], args) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
    else { // parent process
        printf("[%d] %s\n", pxd, args[0]);
    }
}

void handle_signal(int sig) {
    if (sig == SIGTSTP) {
        printf("\n");
        printf("[%d]+ Stopped\n", pid);//simulate a real terminal ctrl z
        if (pid > 0) { // if there's a running process
            kill(pid, SIGINT); // send a SIGINT signal to the child process
            pid = 0; // reset the child process ID
        }
    }
}

void RedirectOutput(char **args) {
    pid_t prd;
    int fd;
    char *output_file = args[2];
    args[1] = NULL;
    args[2] = NULL;

    prd = fork();
    if (prd == -1) {
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    } else if (prd == 0) { // child process
        fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("Failed to open file");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd, STDOUT_FILENO) == -1) { // duplicate the file descriptor
            perror("Failed to redirect output");
            exit(EXIT_FAILURE);
        }
        close(fd);
        execvp(args[0], args); // execute the command
        perror(args[0]);
        exit(EXIT_FAILURE);
    } else { // parent process
        wait(NULL);
    }
}
