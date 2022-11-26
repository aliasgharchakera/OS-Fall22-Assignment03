#include<stdio.h>
#include<stdlib.h>
#include<string.h>
// for open()
#include<fcntl.h> 
#include<errno.h>
// for dup2() and exec()
#include <unistd.h>
// for waitpid()
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

extern int errno;

#include "builtInCMDs.c"
#pragma once

void printHelp(){
    printf("Help:\n");
    printf("jobs - provides a list of all background processes and their local pIDs enumerated from 1\n");
    printf("history - prints a list of previously executed commands\n");
    printf("cd PATHNAME - sets the PATHNAME as working directory.\n");
    printf("exit - terminates the shell only if there are no background jobs.\n");
    printf("kill PID - terminates the background process identified locally with PID in the jobs list.\n");
    printf("!CMD - runs the command numbered CMD in the command history.\n");
    printf("help - prints the list of builtin commands along with their description.\n");
}

void changeDirectory(char* path){
    struct stat sb;
    if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
    {
        chdir(path);
    }
    else {
        printf("Directory does not exist\n");
    }
}

void printHistory(parseInfo* history[10], int runningIndex){
    printf("History:\n");
    for (int i = 0; i < 10; i++){
        // if (history[i] != NULL){
            // printf("runningIndex: %d\n", runningIndex);
            printf("%d: %s\n", i+1, history[runningIndex % (runningIndex%10) + i]->CommArray->command);
            
        // } else {
        //     printf("You haven't typed commands before.\n");
        // }
    }
}

void printJobs(parseInfo* backgroundProcesses[10], int backgroundIndex){
    printf("Jobs:\n");
    for (int i = 0; i < 10; i++){
        if (backgroundProcesses[i] != NULL){
            // printf("runningIndex: %d", runningIndex);
            printf("%d: %s\n", i+1, backgroundProcesses[i]->CommArray->command);
        } else {
            printf("No More Jobs Allocated..\n");
        }
    }
}

void executePrevious(parseInfo* history[10], char command[], int runningIndex, int backgroundIndex, parseInfo* backgroundProcesses[10]){
    char num = (int) command[1];
    if (command[1]=='1' && command[0]=='0'){
        num = 10+48;
    }
    int index = runningIndex % (runningIndex%10) + (num - 48) - 1;
    printf("Executing last %d command from history which is %s \n", index+1, history[index]->CommArray->command);
    
    if (!isBuiltin(history[index]->CommArray->command)){
        if (execvp(history[index]->CommArray->command, history[index]->CommArray->VarList) == -1) {
            // Entering an ambiguous command in which case exec() will return.
            fprintf(stderr, "Error: %s\n", strerror(errno));
        } else{
            execvp(history[index]->CommArray[0].command, history[index]->CommArray[0].VarList);
        }
    } else {
        if (strcmp(history[index]->CommArray[0].command, "jobs") == 0) {
            printJobs(backgroundProcesses, backgroundIndex);
        } else if (strcmp(history[index]->CommArray[0].command, "history") == 0) {
            printHistory(history, runningIndex);
        } else if (strcmp(history[index]->CommArray[0].command, "cd") == 0) {
            changeDirectory(history[index]->CommArray[0].VarList[1]);
        } else if (strcmp(history[index]->CommArray->command, "help") == 0) {
            printHelp();
        } else if (strcmp(history[index]->CommArray->command, "kill") == 0) {
            kill(history[index]->CommArray->VarList[1], SIGKILL);
        }
    }
}

int isBuiltin(char* command){
    if (strcmp(command, "jobs") == 0 || strcmp(command, "history") == 0 || strcmp(command, "cd") == 0 || strcmp(command, "kill") == 0 || strcmp(command, "help") == 0 || strcmp(command, "exit") == 0 || command[0] == '!'){
        return 1;
    }
    else return 0;
}

void terminateAndRemove(int sigchild, parseInfo* backgroundProcesses[10], int backgroundIndex){
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);
    if (pid > 0){
        printf("Process %d terminated with status %d", pid, status);
        backgroundProcesses[backgroundIndex] = NULL;
        printf("Removed from background processes");
        printJobs(backgroundProcesses, backgroundIndex);
    }
}

int main() {

    int pd[2];
    pipe(pd);

    parseInfo* history[10];
    int index = 0;

    parseInfo* backgroundProcesses[10];
    int backgroundProcessesIndex = 0;

    int remainingJobs = 0; 

    while (1) {
// Display prompt
        printf("[%s %s]:~$ ", getlogin(), getcwd(NULL, 0));
// command command
        char *command = (char *)malloc(100);
        fgets(command, MAXLINE, stdin);
// Parse the command
        parseInfo *info = parse(command);
        
        if(info->CommArray->command[0]=='!'){
            history[index] = history[index % (index%10) + ((int) info->CommArray->command[1] - 48) - 1];
        }
        else{
            history[index] = info;
        }
        

        index = (index + 1) % 10;

        if (strcmp(info->CommArray->command, "exit") == 0) {
                if (remainingJobs>0){
                    printf("There are still background processes running. Please wait for them to finish.\n");
                }
                else{
                    exit(0);
                }
        }

        int pid = fork();
        if (pid == 0) {
            if (info->boolOutfile){
                if (access(info->outFile, F_OK)!=0){
                    printf("File does not exist\n");
                    continue;
                }
                if (access(info->outFile, W_OK)!=0){
                    printf("File isn't writable.\n");
                    continue;
                }
                int outfile_desc = open(info->outFile, O_WRONLY);
                dup2(outfile_desc, STDOUT_FILENO);
            }

            if (info->boolInfile){
                if (access(info->inFile, F_OK)!=0){
                    printf("File does not exist\n");
                    continue;
                }
                if (access(info->inFile, R_OK)!=0){
                    printf("File is not readable\n");
                    continue;
                }
                int infile_desc = open(info->inFile, O_RDONLY);
                dup2(infile_desc, STDIN_FILENO);
            }

            if(info->boolBackground){
            
                remainingJobs++;
                backgroundProcesses[backgroundProcessesIndex] = info;
                backgroundProcessesIndex = (backgroundProcessesIndex + 1) % 10;
                int ppid = fork();
                if (ppid == 0){
                    if(!isBuiltin(info->CommArray->command)){
                        execvp(info->CommArray->command, info->CommArray->VarList);
                        printf("Command not found\n");
                        exit(0);
                    }
                    else
                    {
                        if (info->CommArray->command[0] == '!') {
                            executePrevious(history, index, info->CommArray->command[1], backgroundProcesses, backgroundProcessesIndex);
                        } 
                        if (strcmp(info->CommArray[0].command, "history") == 0) {
                            printHistory(history, index);
                        }
                        if (strcmp(info->CommArray[0].command, "cd") == 0) {
                            struct stat sb;
                            if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
                            {
                                chdir(path);
                            }
                            else {
                                printf("Directory does not exist\n");
    }
                        } 
                        if (strcmp(info->CommArray->command, "help") == 0) {
                            printf("Help:\n");
                            printf("jobs - provides a list of all background processes and their local pIDs enumerated from 1\n");
                            printf("history - prints a list of previously executed commands\n");
                            printf("cd PATHNAME - sets the PATHNAME as working directory.\n");
                            printf("exit - terminates the shell only if there are no background jobs.\n");
                            printf("kill PID - terminates the background process identified locally with PID in the jobs list.\n");
                            printf("!CMD - runs the command numbered CMD in the command history.\n");
                            printf("help - prints the list of builtin commands along with their description.\n");
                        } 
                        if (strcmp(info->CommArray[0].command, "jobs") == 0) {
                            printJobs(backgroundProcesses, backgroundProcessesIndex);
                        } 
                        if (strcmp(info->CommArray->command, "kill") == 0) {
                            kill(info->CommArray->VarList[1], SIGKILL);
                        } 
                            
                    }                   
                }
                else{
                    signal(SIGCHLD, SIGKILL);
                    waitpid(ppid, NULL, WNOHANG);
                    backgroundProcesses[backgroundProcessesIndex] = NULL;
                    remainingJobs--;
                }
            }

            if (strcmp(info->CommArray->command, "jobs") == 0) {
                printJobs(backgroundProcesses, backgroundProcessesIndex);
            } else if (strcmp(info->CommArray->command, "history") == 0) {
                printHistory(history, index);
            } else if (strcmp(info->CommArray->command, "cd") == 0) {
                changeDirectory(info->CommArray->VarList[1]);
            } else if (strcmp(info->CommArray->command, "kill") == 0) {
                kill(info->CommArray->VarList[1], SIGKILL);
            } else if (info->CommArray->command[0] == '!') {
                executePrevious(history, info->CommArray->command, index, backgroundProcessesIndex, backgroundProcesses);
            } else if (strcmp(info->CommArray->command, "help") == 0) {
                printHelp();
            }

            else if (execvp(info->CommArray[0].command, info->CommArray[0].VarList) == -1) {
                fprintf(stderr, "Error: %s\n", strerror(errno));
            }
            else{
                execvp(info->CommArray[0].command, info->CommArray[0].VarList);
            }
        }
    
    else {
            waitpid(pid, NULL, 0);
        }
    }
}