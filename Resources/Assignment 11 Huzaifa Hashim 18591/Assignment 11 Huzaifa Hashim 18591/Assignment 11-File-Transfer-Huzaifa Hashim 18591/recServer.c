// Huzaifa Hashim - 18591

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> /* close */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define MAX_THREAD 5
#include <pthread.h>


#define SUCCESS 0
#define ERROR   1

#define END_LINE 0x0
#define SERVER_PORT 1500
#define MAX_MSG 100

 int threadNo = 0;
 bool isFileName = false;
 char fileBuf[20];
 char fileBufCpy[20];
 
 char* sourceFileName;
 char* sourceFileFormat;

struct params
{
  int threadNum;
  char file_name[20];
  char file_format[4];
  int newSd;
  void* threadBuffer[62];
};

/* function readline */
int read_line();

void* chunkin(void* input)
{
       
	char filename[20];

	char file[20];
	char fileCpy[20];
	
	int newSd1 = ((struct params*)input)->newSd;
	void * threadBuf = ((struct params*)input)->threadBuffer;

	strcpy(fileCpy,sourceFileName);
	sprintf(fileCpy, "sf%d.", threadNo);
	strcat(fileCpy,((struct params*)input)->file_format);
	
	printf("thread complete.\n");
	threadNo++;
       
	int drc = open(fileCpy, O_CREAT | O_WRONLY, 0777);
        int count=0;	

	
        bzero(threadBuf, 62);
	int len = 0;
        len = read(newSd1,threadBuf,62); // read from socket
	  write(drc,threadBuf,len);
	    
	bzero(threadBuf, 62);
	
}

int main (int argc, char *argv[]) {
  
  int sd, newSd1, cliLen;

  struct sockaddr_in cliAddr, servAddr;


  /* create socket */
  sd = socket(AF_INET, SOCK_STREAM, 0);
   if(sd<0) {
    perror("cannot open socket ");
    return ERROR;
  }
  
  /* bind server port */
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(SERVER_PORT);
  
  if(bind(sd, (struct sockaddr *) &servAddr, sizeof(servAddr))<0) {
    perror("cannot bind port ");
    return ERROR;
  }

  listen(sd,5);
  
  pthread_t threads[MAX_THREAD];
  int i = 0;
 
  struct params* args = (struct params*)malloc(sizeof(struct params));
  //threadNo = (int)(args->threadNum);
 // args->threadNum = args->threadNum + 1;

    
  bzero(fileBuf, 20);
bzero(fileBufCpy, 20);


 

  while(1) {

    printf("%s: waiting for data on port TCP %u\n",argv[0],SERVER_PORT);

    cliLen = sizeof(cliAddr);
    newSd1 = accept(sd, (struct sockaddr *) &cliAddr, &cliLen);
    if(newSd1<0) {
      perror("cannot accept connection ");
      return ERROR;
    }
    
   if (isFileName == false){
	read(newSd1,fileBuf,20);
        isFileName = true;
        close(newSd1);
        printf("File name is: %s\n",fileBuf);
	// get file format
  	 strcpy(fileBufCpy, fileBuf);
         sourceFileName = strtok(fileBufCpy, ".");
         sourceFileFormat = strtok(NULL,".");
	 strcpy(args->file_format,sourceFileFormat);
        continue;

}
   


    args->newSd = newSd1;
    // create threads and join them

  
      pthread_create(&threads[i], NULL, &chunkin, (void *)args);

      int retval = pthread_join(threads[i], NULL);
	if(retval == -1)
	perror("thread join error");
      i++;

      close(newSd1);
      if ( i == 5) break;
	
    
  } /* while (1) */
  

 
  //  after all the threads are done
	printf("Working, please wait...\n");
	char mergeFileName[20];
	strcpy(mergeFileName, "merge.");
	strcat(mergeFileName,sourceFileFormat);
	printf("Merge file name is: %s\n", mergeFileName);
	printf("Merging, please wait...\n");

	int dfd = open(mergeFileName, O_CREAT | O_WRONLY, 0777);
	if( dfd == -1){
		printf("Error opening file.");
	}

        char filename[20];
 for (i = 0; i < MAX_THREAD; i++) {
	sprintf(filename, "sf%d.", i);
	strcat(filename, sourceFileFormat);

	int sfd = open(filename, O_RDONLY);
	int len = 0;
	void* buf[20];

	
        while (len = read(sfd,buf,1) != 0){
	//write(STDOUT_FILENO,buf,len); // keep this for printing text files only
	write(dfd,buf,1);
	}
       close(sfd);
 }
	printf("File merged successfully.\n");
       close(dfd); 

}



 
  
// Huzaifa Hashim - 18591