#include <math.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <dirent.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>


#define SUCCESS 0
#define ERROR   1

#define END_LINE 0x0
#define SERVER_PORT 1500
#define MAX_MSG 100

//Huzaifa Hashim 18591	
char ls[10] = "ls";
char l[10] = "-l";
char a[10] = "-a";
char R[10] = "-R";

bool isInvalid = false;


void flagR(char buf[], char *name, int indent){
    
    DIR *dir;
    struct dirent *entry;
    if (!(dir = opendir(name)))
        return;
    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || 
		strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
	   int i;
	  for (i = 0; i < indent; i++) 
	{
	strcat(buf," ");
	}
	strcat(buf,"[");
	strcat(buf, entry->d_name);
	strcat(buf,"]");
	strcat(buf, "\n");
            flagR(buf, path, indent + 2);
        } else {
	int i;
	  for (i = 0; i < indent; i++) 
	{
	strcat(buf," ");
	}
	strcat(buf, entry->d_name);
	strcat(buf, "\n");
        }
    }
    closedir(dir);
}


void flagL(char buf[], char* directory) {
strcpy(buf,"\n");
DIR *dp;
struct passwd *pws;
char *c;
struct group *gp;
struct dirent *ep;
struct stat f;
dp = opendir(directory);
stat(directory, &f);
while ((ep=readdir(dp))!=NULL){
stat(ep->d_name,&f);

(S_ISDIR(f.st_mode))? strcat(buf,"d"):strcat(buf,"-");
(f.st_mode & S_IRUSR) ? strcat(buf,"r"):strcat(buf,"-");
(f.st_mode & S_IWUSR) ? strcat(buf,"w"):strcat(buf,"-");
(f.st_mode & S_IXUSR) ? strcat(buf,"x"):strcat(buf,"-");
(f.st_mode & S_IRGRP) ? strcat(buf,"r"):strcat(buf,"-");
(f.st_mode & S_IWGRP) ? strcat(buf,"x"):strcat(buf,"-");
(f.st_mode & S_IXGRP) ? strcat(buf,"x"):strcat(buf,"-");
(f.st_mode & S_IROTH) ? strcat(buf,"r"):strcat(buf,"-");
(f.st_mode & S_IWOTH) ? strcat(buf,"w"):strcat(buf,"-");
(f.st_mode & S_IXOTH) ? strcat(buf,"x"):strcat(buf,"-");

strcat(buf," ");


char fstnlink[10];
snprintf(fstnlink, sizeof(fstnlink), "%d", f.st_nlink);
strcat(buf, fstnlink);
strcat(buf," ");

pws = getpwuid(f.st_uid);

strcat(buf, pws->pw_name);
strcat(buf," ");

gp=getgrgid(f.st_gid);

strcat(buf, gp->gr_name);
strcat(buf," ");
char fstsize[10];
snprintf( fstsize, sizeof(fstsize), "%d", f.st_size);
strcat(buf, fstsize);
strcat(buf," ");

strcat(buf,ctime(&f.st_mtime));

strcat(buf, ep->d_name);
strcat(buf, "\n");

}
closedir(dp);

}


void flagA(char buf[], char* directory){
char name[100];
strcpy(buf,"\n");
DIR *dp;
struct dirent *ep;
dp = opendir(directory);
if (dp !=NULL){
	while(ep = readdir(dp)) {
		
		char type[10];
		if (ep->d_type == DT_DIR)
		strcpy(type,"Dir");
		else
		strcpy(type,"File");
	
	strcat(buf, type);
        strcat(buf, ": ");
        strcat(buf,ep->d_name);
        strcat(buf,"\n");
	}
closedir(dp);
}
else {
puts("Can't open this directory.");
}

}

void lsOnly(char buf[], char* directory){ 
bzero(buf, 100000);
strcpy(buf,"\n");
char name[100];
DIR *dp;
struct dirent *ep;
dp = opendir(directory);
if (dp !=NULL){
	while(ep = readdir(dp)) {
		strcpy(name,ep->d_name);
		if ((strcmp(name, ".") == 0) || (strcmp(name, "..") == 0) || (name[0] =='.')) { 
		
		}
else{
		char type[10];
		if (ep->d_type == DT_DIR)
		strcpy(type,"Dir");
		else
		strcpy(type,"File");
	
	strcat(buf, type);
        strcat(buf, ": ");
        strcat(buf,ep->d_name);
        strcat(buf,"\n");
}
	}
closedir(dp);
}
else {
puts("Can't open this directory.");
}

}



void error(const char*msg){

perror(msg);
exit(1);
}
int main (int argc, char *argv[]) {
  char buffer[100000];
char* description;
  int sd, newSd, cliLen, n;

  struct sockaddr_in cliAddr, servAddr;



  /* creating socket */
  sd = socket(AF_INET, SOCK_STREAM, 0);
   if(sd<0) {
    perror("Can't open socket ");
    return ERROR;
  }
  
  /* binding server port */
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(SERVER_PORT);
  
  if(bind(sd, (struct sockaddr *) &servAddr, sizeof(servAddr))<0) {
    perror("Can't bind port ");
    return ERROR;
  }

  listen(sd,5);
  
  while(1) {
    printf("%s: Waiting for data on port TCP %u\n",argv[0],SERVER_PORT);

    cliLen = sizeof(cliAddr);
    newSd = accept(sd, (struct sockaddr *) &cliAddr, &cliLen);
    if(newSd<0) {
      perror("Connection cannot be accepted");
      return ERROR;
    }
    
   bzero(buffer, 100000);
   n = read(newSd, buffer, 100000);
   if (n <0) error("Reading error");
   if(strcmp(buffer, "Huzaifa 1234")!=0)
   {
   strcpy(buffer,"Incorrect credentials, closing client connection.\n");
   n = write(newSd, buffer, strlen(buffer));
   bzero(buffer, 100000);
continue;
   }
   else {
	strcpy(buffer,"Credentials verified. Enter your ls command.\n");
   n = write(newSd, buffer, strlen(buffer));
   bzero(buffer, 100000);
} 
   
    
    while(1) {
      bzero(buffer, 100000);
      n = read(newSd, buffer, 100000);
      if (n <0) error("Reading error");
	printf("Client: %s\n", buffer);

char buffCpy[100000];
	strcpy(buffCpy, buffer);
	bzero(buffer, 100000);
	strcpy(buffer,"\n");
	char* piece = strtok(buffCpy, " ");
	while(piece != NULL) 
	{ if (strcmp(piece, ls) ==0) {
	piece = strtok(NULL, " ");
	if (piece == NULL) 
		lsOnly(buffer,"./");
	else {
		if (strcmp(piece, l) ==0){
       	piece = strtok(NULL, " ");
	(piece == NULL)? flagL(buffer,"./"):flagL(buffer,piece);
	}
		else if (strcmp(piece, R) ==0){
       	 piece = strtok(NULL, " ");
	(piece == NULL)? flagR(buffer,"./",0):flagR(buffer,piece,0);
	}
	else if (strcmp(piece, a) ==0){
       piece = strtok(NULL, " ");
	(piece == NULL)? flagA(buffer,"./"):flagA(buffer,piece);
	}
	else { lsOnly(buffer, piece);}

	} 
	}
	
	else if (strcmp(piece, "exit") == 0) {
	bzero(buffer,100000);
	isInvalid = true;
       strcpy(buffer,"Client closed the connection\n");
	n = write(newSd, buffer, strlen(buffer));
	break;
	}
	else {
      bzero(buffer, 100000);
	isInvalid = true;
      strcpy(buffer,"Invalid argument. Closing client connection\n");
	n = write(newSd, buffer, strlen(buffer));
      	break;
	}
	piece = strtok(NULL, " ");
	}

	int i = strncmp("Connection closed by client.\n", buffer, 30);
      if (i==0 || isInvalid == true) { close(newSd);break;}

	n = write(newSd, buffer, strlen(buffer));
	if (n <0) error("Typing error");
      
    }
    
  }
//Huzaifa Hashim 18591
close(sd);
return 0;
}

  
