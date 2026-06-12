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
#define COLOR_GREEN "\033[1;32m"
#define COLOR_BLUE  "\033[1;34m"
#define COLOR_RESET "\033[0m"

volatile sig_atomic_t childIsRunning = 0;
void moveToNewLine(int sig)
{
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
        // Print the identical colored prompt structure here!
        printf("\n%szaid@custom-shell%s:%s%s%s> ", 
               COLOR_GREEN, COLOR_RESET, 
               COLOR_BLUE, cwd, COLOR_RESET);
    }
    fflush(stdout);
}
int main(int argc, char* argv[])
{
    signal(SIGINT, moveToNewLine);
    while(1)
    {
        char str[1024];
        char* args[10];
        char* args_left[10];
        char* args_right[10];
        int fd[2];
        bool pipeHandling = false;
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            strcpy(cwd, "shell"); 
        }

        printf("%szaid@custom-shell%s:%s%s%s> ", 
               COLOR_GREEN, COLOR_RESET, 
               COLOR_BLUE, cwd, COLOR_RESET);
        fflush(stdout);
        if(fgets(str, sizeof(str), stdin) == NULL)
        {
            clearerr(stdin);
            continue;
        }
        int lineTerminator = strcspn(str, "\n");
        str[lineTerminator] = '\0';

        char* piece = strtok(str, " ");
        int i = 0;
        while(piece != NULL)
        {
            args[i] = piece;
            piece = strtok(NULL, " ");
            i++;
        }
        args[i] = NULL;
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

        if(pipeHandling)
        {
            if (pipe(fd) == -1)
            {
                printf("Error please try again");
                return 3;
            }
            pid_t pid = fork();
            if (pid == 0)
            {
                //first child will handle part before |
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                close(fd[0]);
                execvp(args_left[0], args_left);
            }
            else
            {
                //second child will handle part after |
                pid_t second_id = fork();
                if(second_id == 0)
                {
                    dup2(fd[0], STDIN_FILENO);
                    close(fd[0]);
                    close(fd[1]);
                    execvp(args_right[0], args_right);
                }
                else
                {
                    close(fd[0]);
                    close(fd[1]);
                    waitpid(pid, NULL, 0);
                    waitpid(second_id, NULL, 0);
                }
            }

            continue;
        }
        
        if (args[0] == NULL)
        {
            continue;
        }

        if (strcmp(args[0], "exit") == 0)
        {
            printf("Exiting...\n");
            return 2;
        }

        if(strcmp(args[0], "cd") == 0)
        {
            if(args[1] == NULL)
            {
                printf("Expected argument to \"cd\"\n");
            }
            else
            {
                if(chdir(args[1]) == -1)
                {
                    perror("cd failed");
                }
            }
            continue;
        }

        childIsRunning = 1;
        int id = fork();
        if (id == -1)
        {
            printf("Error please try again.");
            childIsRunning = 0;
            return 1;
        }
        else if(id == 0)
        {
            signal(SIGINT, SIG_DFL);
            if(execvp(args[0], args) == -1)
            {
                printf("Command '%s' not found \n", args[0]);
                exit(127);
            }
            exit(1);
        }
        else
        {
            wait(NULL);
            childIsRunning = 0;
        }

        
        }
    return 0;
}