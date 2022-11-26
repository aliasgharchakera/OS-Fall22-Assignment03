#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parse.c"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


void help();
void change_d(char *path);
void exit();
int pipes (int n, struct commandType cmd[]);
int infile;
int outfile;

int main(int argc, char *argv[]) 
{   
    while(1){

        printf("batman $ ");
        // char* cmdline = readline("");
        char cmdline[50];
        scanf("%[^\n]%*c", cmdline)
        parseInfo *p = parse(cmdline);
       
        int rc = fork();
        
       if (rc == 0) {
            if (p->pipeNum>0){
                pipes(p->pipeNum, p->CommArray);
            }
            else{

                if(p->boolInfile){
                    infile = open(p->inFile, O_RDONLY);
                    dup2(infile, fileno(stdin));
                    close(infile);
                }
                if (p->boolOutfile){
                    outfile = open(p->outFile, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
                    dup2(outfile, fileno(stdout));
                    close(outfile);
                }
                
              
                 else
				{
                    int argNum = p->CommArray[0].VarNum;
                    char *myargs[MAX_VAR_NUM] = {NULL};
                    myargs[0] = p->CommArray[0].command;   
                    for (int i = 0;  i <=argNum;i++){
                                myargs[i] = p->CommArray[i].VarList[i-1];
                            }
                    if (strcmp(p->CommArray[0].command,"exit")==0 ){
                        exit(1);
                        return 0;}
                
                    if (strcmp(myargs[0], "help") == 0 ){help();}
                    if (strcmp(myargs[0], "cd") == 0 ){
                        change_d(p->CommArray[0].VarList[0]);}
                    else{
                        execvp(myargs[0], myargs)
                        

                    }
				}
            }
        
            
        }else {
            
                wait(NULL);

            
        }
    }

    return 0;
}


int pipes (int n, struct commandType cmd[])
{
  int i;
  int pid;
  int in, fd [2];
  in = 0;
  for (i = 0; i <= n - 1; ++i)
    {
      pipe (fd);
      int infile = in;
      int outfile = fd[1];
      int pid;
    if ((pid = fork ()) == 0)
        {
        if (infile != 0)
            {
            dup2 (infile, 0);
            close (infile);
            }

        if (outfile != 1)
            {
            dup2 (outfile, 1);
            close (outfile);
            }

        execvp (cmd[i].VarList[0],cmd[i].VarList);
        }


      close (fd [1]);
      in = fd [0];
    }
  if (in != 0)
    dup2 (in, 0);
    
  return execvp (cmd[i].VarList [0], cmd[i].VarList);
}
void help(){
    printf("1. jobs - provides a list of all background processes and their local pIDs.\n");
    printf("2. cd PATHNAME - sets the PATHNAME as working directory.\n");
    printf("3. history - prints a list of previously executed commands. Assume 10 as the maximum history\n");
    printf("4. kill PID - terminates the background process identified locally with PID in the jobs list.\n");
    printf("5. !CMD - runs the command numbered CMD in the command history.\n");
    printf("6. exit - terminates the shell only if there are no background jobs.\n");
    printf("7. help - prints the list of builtin commands along with their description.\n");
}
void cd(char *path){
  
    chdir(path);
}