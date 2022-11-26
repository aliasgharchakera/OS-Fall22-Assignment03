#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include "parse.c"
#include <dirent.h>
#include <errno.h>
#include <wait.h>

volatile processes tasks[10];
volatile int totalTasks = 0;


void signal_handler(int signal)
{
    int *status;
    pid_t child_id;
    
    if ((child_id = waitpid(-1, &status, WNOHANG)) > 0)
    {
        printf("Child %ld terminated\n", child_id);
        for (int i = 0; i < totalTasks; i++)
        {
            //if child id is equal to kill id, then remove that task from tasks array, and decrement total task. 
            if (tasks[i].pid == child_id)
            {
                for (int j = i; j < totalTasks - 1; j++)
                {
                    tasks[j] = tasks[j + 1];
                }
            }
            //
            totalTasks = totalTasks - 1;
        }
    }
}

typedef struct
{
    pid_t pid;
    int Local_ID;

} processes;
int main()
{
    //command shell starting. 
    // printf("Command Shell Initiated");
    // printf("\n");
    
    //it will store the commands executed. 
    parseInfo *commandsList[10];
    int commandsList_total = 0;
    
    while (1)
    {
        char cwd[256];
        // <Display prompt>
        printf("user $ ");
        //this function will rectrieve the current working directory from os function. 
        // getcwd(cwd, sizeof(cwd));

        //print the cwd on screen with # in front of it so the user can enter the prompt.
        // printf("%s # ", cwd);

        char cmd[50];
        
        // user will enter the cmd, and it will be scanned. 
        scanf("%[^\n]%*c", cmd);
        
        //parsing the command. 
        parseInfo *cmdInfo = parse(cmd);
        if (cmdInfo->pipeNum > 0)
        {
           //cmd1 has commands that is in start of the pipe. 
            struct commandType *cmd1 = &(cmdInfo->CommArray[0]);
            //cmd2 has commands that is in the end of pipe. 
            struct commandType *cmd2 = &(cmdInfo->CommArray[1]);

            //it is the first input of the pipe. 
            char *frontPipeCmd[cmd1->VarNum + 1];
            //the output from frontPipeCmd will go into this array. 
            char *pipeExitCmd[cmd2->VarNum + 1];
            
            for (int i = 0; i < cmd1->VarNum; i++)
            {
                //it stores the input from the file. 
                frontPipeCmd[i] = cmd1->VarList[i];
            }
            for (int i = 0; i < cmd2->VarNum; i++)
            {
                pipeExitCmd[i] = cmd2->VarList[i];
            }
            //pipe code from the file. 
            int pd[2];
            frontPipeCmd[cmd1->VarNum] = NULL;
            pipeExitCmd[cmd2->VarNum] = NULL;
            if (pipe(pd) == -1)
            {
                printf("Pipe is not created \n");
                exit(1);
            }
            if (fork() == 0)
            {
                close(pd[0]);
                dup2(pd[1], 1);
                close(pd[0]);
                close(pd[1]);

                if (execvp(frontPipeCmd[0], frontPipeCmd) < 0)
                {
                    printf("cmd 1 not executed \n");
                    exit(0);
                }
            }
            if (fork() == 0)
            {
                close(pd[1]);
                dup2(pd[0], 0);
                close(pd[1]);
                close(pd[0]);

                if (execvp(pipeExitCmd[0], pipeExitCmd) < 0)
                {
                    printf("cmd 2 not executed \n");
                }
                exit(0);
            }
            close(pd[0]);
            close(pd[1]);
        }
        else
        {
            if (cmdInfo->boolBackground == 0 || totalTasks <= 9) 
            {
                if (commandsList_total < 10)
                {
                    commandsList[commandsList_total] = cmdInfo;
                    commandsList_total += 1;
                }
                else
                {
                    for (int i = 0; i < commandsList_total - 1; i++)
                    {
                        commandsList[i] = commandsList[i + 1];
                    }

                    commandsList[commandsList_total - 1] = cmdInfo;
                }

                if (strcmp(cmdInfo->CommArray[0].command, "tasks") == 0)
                {
                    for (int j = 0; j < totalTasks; j++)
                    {
                        printf("process id: %d \n process local id: %d \n ", tasks[j].pid,tasks[j].Local_ID);
                    }
                }
                if (strcmp(cmdInfo->CommArray[0].command, "cd") == 0)
                {
                    DIR *dir = opendir(cmdInfo->CommArray[0].VarList[0]);
                        if (dir)
                        {
                            chdir(cmdInfo->CommArray[0].VarList[0]);
                            closedir(dir);
                        }
                        else
                        {
                            printf("path does not exist! \n");
                        }
                }
                if (strcmp(cmdInfo->CommArray[0].command, "commandsList") == 0)
                {
                    for (int i = 0; i < commandsList_total; i++)
                    {
                        printf("Last command executed  %s \n ", commandsList[i]->CommArray[0].command);
                    }
                }
                if (strcmp(cmdInfo->CommArray[0].command, "kill") == 0)
                {
                    for (int i = 0; i < totalTasks; i++)
                    {
                        if (tasks[i].pid == *cmdInfo->CommArray->VarList[0])
                        {
                            kill(tasks[i].pid, SIGKILL);
                            for (int j = i; j < totalTasks - 1; j++)
                            {
                                tasks[j] = tasks[j + 1];
                            }
                        }
                        totalTasks = totalTasks - 1;
                    }
                }

                if (strcmp(cmdInfo->CommArray[0].command, "help") == 0)
                {
                    printf(
                        "\nList of Commands supported:"
                        "\n>exit - terminates the shell only if there are no background tasks."
                        "\n>tasks - provides a list of all background processes and their local pIDs."
                        "\n>kill PID - terminates the background process identified locally with PID in the tasks list."
                        "\n>cd PATHNAME - sets the PATHNAME as working directory."
                        "\n>commandsList - prints a list of previously executed commands."
                        "\n>!CMD - runs the command numbered CMD in the command commandsList."
                        "\n>help - prints the list of builtin commands along with their description. \n");
                }

                if (strcmp(&(cmdInfo->CommArray[0].command[0]), "!") == 0)
                {
                    int commandsList_number = atoi(&(cmdInfo->CommArray[0].command[1]));
                    execvp(commandsList[commandsList_number - 1]->CommArray->command, commandsList[commandsList_number - 1]->CommArray[0].VarList);
                }

                if (strcmp((cmdInfo->CommArray[0].command), "exit") == 0)
                {
                    if (totalTasks == 0)
                    {
                        printf("Command Shell will terminated\n");
                        sleep(2);
                        exit(1);
                    }
                    else
                    {
                        printf("background tasks remaining \n");
                    }
                }
                else
                {
                    int pid = fork();
                    if (pid == 0)
                    { 
                        int in = open(cmdInfo->inFile, O_RDONLY);
                        dup2(in, STDIN_FILENO);
                        close(in);
                        int out = open(cmdInfo->outFile, O_WRONLY | O_CREAT, 0666); 
                        dup2(out, STDOUT_FILENO);
                        close(out);
                        execvp(cmdInfo->CommArray[0].command, cmdInfo->CommArray[0].VarList);
                    }
                    else
                    {
                        if (cmdInfo->boolBackground == 1)
                        {
                            tasks[totalTasks].pid = pid;
                            tasks[totalTasks].Local_ID = totalTasks;
                            totalTasks = totalTasks + 1;
                            signal(SIGCHLD, signal_handler);
                        }
                        if (cmdInfo->boolBackground == 0)
                        {
                            waitpid(pid,NULL,0);
                        }
                    }
                }
            }
            else
            {
                printf("task limit exceeded! \n");
            }
        }
    }
    return 0;
}