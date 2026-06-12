#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>
//-------------------Included Items------------------------------------------------------------------//
#define COLOR_GREEN "\033[1;32m"
#define COLOR_BLUE  "\033[1;34m"
#define COLOR_RESET "\033[0m"
//------------------Defined colour for UX------------------------------------------------------------//

volatile sig_atomic_t childIsRunning = 0; // global variable for print control

void moveToNewLine(int sig)
{ 
    /*
    This method is used to print the appropriate statements to the next line of the shell.
    */
    if(childIsRunning)
    {
        printf("\n");
    }
    else
    {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            strcpy(cwd, "shell");
        }

        printf("\n%szaid@custom-shell%s:%s%s%s> ", 
               COLOR_GREEN, COLOR_RESET, 
               COLOR_BLUE, cwd, COLOR_RESET);
    }
    fflush(stdout);
}
int main(int argc, char* argv[])
{
    signal(SIGINT, moveToNewLine);
    while(1) //Continuously loop and ask for user input
    {
        char str[1024]; //variable used to obtain user input
        char* args[10]; //number of arguments passed by user
        char* args_left[10]; //number of arguments before pipe '|'
        char* args_right[10]; //number of arguments after pipe '|'
        int fd[2];
        bool pipeHandling = false; //determines whether a pipe is required
        char cwd[1024]; //holds current directory
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            strcpy(cwd, "shell"); 
        }

        printf("%szaid@custom-shell%s:%s%s%s> ", 
               COLOR_GREEN, COLOR_RESET, 
               COLOR_BLUE, cwd, COLOR_RESET);
        fflush(stdout);
        if(fgets(str, sizeof(str), stdin) == NULL)
        {
            //if there is no input then the rest of the program may be skipped
            clearerr(stdin); 
            continue;
        }

        int lineTerminator = strcspn(str, "\n"); 
        str[lineTerminator] = '\0';
        //replaces the line terminator symbol with a string terminator for handling

        char* piece = strtok(str, " ");
        int i = 0;
        while(piece != NULL)
        {
            args[i] = piece;
            piece = strtok(NULL, " ");
            i++;
        }
        args[i] = NULL;
        //obtains user input and stores in an array of strings to pass as arguments

        for (int j = 0; j < i; j++)
        {
            if(strcmp(args[j], "|") == 0)
            {
                pipeHandling = true;
                for(int k = 0; k < j; k++)
                {
                    args_left[k] = args[k];
                }
                args_left[j] = NULL;
                int m = 0;
                for(int l = j + 1; l < i; l++)
                {
                    args_right[m] = args[l];
                    m++;
                }
                args_right[m] = NULL;
            }
        }
        //if there is a pipe method called then the arguments must be split for handling

        if(pipeHandling) //only enter if pipe handling required
        {
            if (pipe(fd) == -1)
            {
                printf("Error please try again");
                return 3;
            }
            pid_t pid = fork(); //one child created from main parent process
            if (pid == 0)
            {
                //first child will handle part before |
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                close(fd[0]);
                execvp(args_left[0], args_left);
                //creates duplicate of fd[1] and stores
                //the left side of the pipe is executed
            }
            else
            {
                //second child will handle part after |
                pid_t second_id = fork(); //second child created from main parent process
                if(second_id == 0)
                {
                    dup2(fd[0], STDIN_FILENO);
                    close(fd[0]);
                    close(fd[1]);
                    execvp(args_right[0], args_right);
                    //creates duplicate of fd[0] and stores
                    //the right side of the pipe is executed
                }
                else
                {
                    close(fd[0]);
                    close(fd[1]);
                    waitpid(pid, NULL, 0);
                    waitpid(second_id, NULL, 0);
                    //parent process closes both ends of the pipe
                    //waits for both child process to finish, prevents zombies
                }
            }

            continue; //the rest of the program may be skipped 
        }
        
        if (args[0] == NULL)
        {
            continue; 
            //the loop may be skipped without user input for command
        }

        if (strcmp(args[0], "exit") == 0)
        {
            //used to exit the shell
            printf("Exiting...\n");
            return 2;
        }

        if(strcmp(args[0], "cd") == 0)
        {
            //handles the change directory command
            if(args[1] == NULL)
            {
                printf("Expected argument to \"cd\"\n");
            }
            else
            {
                if(chdir(args[1]) == -1)
                {
                    //requests a change to a different directory 
                    perror("cd failed");
                }
            }
            continue;
        }

        childIsRunning = 1;
        int id = fork();
        //process is forked so child can execute the requested command and parent can preserve program
        if (id == -1)
        {
            printf("Error please try again.");
            childIsRunning = 0;
            return 1;
        }
        else if(id == 0)
        {
            signal(SIGINT, SIG_DFL); //if CTRL C signal is received
            if(execvp(args[0], args) == -1)
            {
                //attempt to execute the process
                printf("Command '%s' not found \n", args[0]);
                exit(127);
            }
            exit(1);
        }
        else
        {
            //parent waits for child to prevent zombie
            wait(NULL);
            childIsRunning = 0;
        }

        
        }
    return 0;
}
