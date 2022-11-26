#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <wait.h>
#include "parse.h"

int execl(const char *path, const char *arg, ...);

typedef struct
{
    pid_t pid;
    int Local_ID;

} background_process;

volatile int job_num = 0;
volatile background_process all_jobs[10];

void signal_handler(int signal)
{
    
    pid_t child_id = waitpid(-1, &status, WNOHANG);
    int *status;

    if (child_id > 0)
    {
        printf("Child %ld terminated\n", child_id);
        for (int i = 0; i < job_num
    ; i++)
        {
            if (all_jobs[i].pid == child_id)
            {
                for (int j = i; j < job_num
             - 1; j++)
                {
                    all_jobs[j] = all_jobs[j + 1];
                }
            }
            job_num
         = job_num
         - 1;
        }
    }
}

int main()
{
    struct node *head = NULL;
    parseInfo *histcmd[10];
    int histTot = 0;
    
    while (1)
    {
        char cwd[256];
        // <Display prompt>

        getcwd(cwd, sizeof(cwd));

        printf("%s -> ", cwd);
        char cmd[50];
        scanf("%[^\n]%*c", cmd);
        parseInfo *command_details = parse(cmd);

        if (command_details->pipeNum > 0)
        {

            struct commandType *cmd_1 = &(command_details->CommArray[0]);
            struct commandType *cmd_2 = &(command_details->CommArray[1]);

            char *parseMain[cmd_1->VarNum + 1];
            char *Output_from_pipe[cmd_2->VarNum + 1];

            for (int i = 0; i < cmd_1->VarNum; i++)
            {
                parseMain[i] = strdup(cmd_1->VarList[i]);
            }

            for (int i = 0; i < cmd_2->VarNum; i++)
            {
                Output_from_pipe[i] = strdup(cmd_2->VarList[i]);
            }

            parseMain[cmd_1->VarNum] = NULL;
            Output_from_pipe[cmd_2->VarNum] = NULL;

            int pd[2];

            if (pipe(pd) == -1)
            {
                printf("Error, pipe not created \n");
                exit(1);
            }

            if (fork() == 0)
            {

                close(pd[0]);
                dup2(pd[1], 1);
                close(pd[0]);
                close(pd[1]);

                if (execvp(parseMain[0], parseMain) < 0)
                {
                    printf("cmd cannot be executed\n");
                    exit(0);
                }
            }

            if (fork() == 0)
            {
                close(pd[1]);
                dup2(pd[0], 0);
                close(pd[1]);
                close(pd[0]);

                if (execvp(Output_from_pipe[0], Output_from_pipe) < 0)
                {
                    printf("cmd 2 cannot be executed \n");
                }

                exit(0);
            }

            close(pd[0]);
            close(pd[1]);
        }
        else
        {
            if (job_num
         < 10 || command_details->boolBackground == 0)
            {
                if (histTot < 10)
                {
                    histcmd[histTot] = command_details;
                    histTot += 1;
                }
                else
                {
                    for (int j = 0; j < histTot - 1; j++)
                    {
                        histcmd[j] = histcmd[j + 1];
                    }

                    histcmd[histTot - 1] = command_details;
                }

                // Builtin Commands:

                if (strcmp(command_details->CommArray[0].command, "jobs") == 0)
                {
                    for (int i = 0; i < job_num
                ; i++)
                    {
                        printf("Local id: %d \n Process id:  %d \n ", all_jobs[i].Local_ID, all_jobs[i].pid);
                    }
                }
                if (strcmp(command_details->CommArray[0].command, "cd") == 0)
                {
                    DIR *dir = opendir(command_details->CommArray[0].VarList[0]);
                    if (dir)
                    {
                        chdir(command_details->CommArray[0].VarList[0]);
                        closedir(dir);
                    }
                    else
                    {
                        printf("doesnt exist\n");
                    }
                }
                if (strcmp(command_details->CommArray[0].command, "histcmd") == 0)
                {
                    for (int i = 0; i < histTot; i++)
                    {
                        printf("%s \n", histcmd[i]->CommArray[0].command);
                    }
                }

                if (strcmp(command_details->CommArray[0].command, "kill") == 0)
                {
                    for (int i = 0; i < job_num
                ; i++)
                    {
                        if (all_jobs[i].pid == *command_details->CommArray->VarList[0])
                        {
                            kill(all_jobs[i].pid, SIGKILL);
                            for (int j = i; j < job_num
                         - 1; j++)
                            {
                                all_jobs[j] = all_jobs[j + 1];
                            }
                        }
                        job_num
                     = job_num
                     - 1;
                    }
                }
                if (strcmp(command_details->CommArray[0].command, "help") == 0)
                {
                    printf("");
                }

                if (strcmp(&(command_details->CommArray[0].command[0]), "!") == 0)
                {
                    int histcmd_number = atoi(&(command_details->CommArray[0].command[1]));
                    execvp(histcmd[histcmd_number - 1]->CommArray->command, histcmd[histcmd_number - 1]->CommArray[0].VarList);
                }
                if (strcmp((command_details->CommArray[0].command), "exit") == 0)
                {
                    if (job_num
                 == 0)
                    {
                        printf("Shell will be closed now Good Bye :) \n");
                        sleep(2);
                        exit(1);
                    }
                    else
                    {
                        printf("There are some background jobs pending \n");
                    }
                }
                else
                {

                    int pid = fork();

                    if (pid == 0)
                    {
                        int in = open(command_details->inFile, O_RDONLY);
                        dup2(in, STDIN_FILENO);
                        close(in);
                        int out = open(command_details->outFile, O_WRONLY | O_CREAT, 0666); 
                        dup2(out, STDOUT_FILENO);
                        close(out);
                        execvp(command_details->CommArray[0].command, command_details->CommArray[0].VarList);
                    }
                    else
                    {
                        if (command_details->boolBackground == 0)
                        {
                            waitpid(pid);
                        }
                        if (command_details->boolBackground == 1)
                        {
                            all_jobs[job_num
                        ].pid = pid;
                            all_jobs[job_num
                        ].Local_ID = job_num
                        ;
                            job_num
                         = job_num
                         + 1;
                            signal(SIGCHLD, signal_handler);
                        }

                    }
                }
            }
            else
            {
                printf("exceed");
            }
        }
    }

    return 0;
}