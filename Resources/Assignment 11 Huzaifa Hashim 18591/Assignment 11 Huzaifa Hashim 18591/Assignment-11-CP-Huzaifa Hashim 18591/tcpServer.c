#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <fcntl.h>

#define SUCCESS 0
#define ERROR   1

#define END_LINE 0x0
#define SERVER_PORT 1500
#define MAX_MSG 1024

//Huzaifa Hashim 18591
 
int read_line();


int main (int argc, char *argv[]) {
  
  int sd, newSd, cliLen;

  struct sockaddr_in cliAddr, servAddr;
  char line[MAX_MSG];


  sd = socket(AF_INET, SOCK_STREAM, 0);
   if(sd<0) {
    perror("cannot open socket ");
    return ERROR;
  }
  
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(SERVER_PORT);
  
  if(bind(sd, (struct sockaddr *) &servAddr, sizeof(servAddr))<0) {
    perror("cannot bind port ");
    return ERROR;
  }

  listen(sd,5);

int count = 1;

int n;
void* buffer[20];
int dfd = open("destination.txt", O_CREAT | O_WRONLY, 0777);
if (dfd == -1) {
printf("Can't open destination file.");
}


  while(1) {

    printf("%s: waiting for data on port TCP %u\n",argv[0],SERVER_PORT);

    cliLen = sizeof(cliAddr);
    newSd = accept(sd, (struct sockaddr *) &cliAddr, &cliLen);
    if(newSd<0) {
      perror("cannot accept connection ");
      return ERROR;
    }

    while(1){
    n = recv(newSd, buffer, 20, 0);
    if (n ==0){
     printf("Data written in file successfully.\n");
     close(dfd);
     close(newSd);
     break;}
     write(dfd, buffer, 1);
     bzero(buffer, sizeof(buffer));
    } 
  } 


return 0;
}


int read_line(int newSd, char *line_to_return) {
  
  static int rcv_ptr=0;
  static char rcv_msg[MAX_MSG];
  static int n;
  int offset;

  offset=0;

  while(1) {
    if(rcv_ptr==0) {

      memset(rcv_msg,0x0,MAX_MSG); 
      n = recv(newSd, rcv_msg, MAX_MSG, 0); 
      if (n<0) {
	perror(" cannot receive data ");
	return ERROR;
      } else if (n==0) {
	printf(" connection closed by client\n");
	close(newSd);
	return ERROR;
      }
    }
  

    while(*(rcv_msg+rcv_ptr)!=END_LINE && rcv_ptr<n) {
      memcpy(line_to_return+offset,rcv_msg+rcv_ptr,1);
      offset++;
      rcv_ptr++;
    }
    
    if(rcv_ptr==n-1) { 
      *(line_to_return+offset)=END_LINE;
      rcv_ptr=0;
      return ++offset;
    } 
    
    if(rcv_ptr <n-1) {
      *(line_to_return+offset)=END_LINE;
      rcv_ptr++;
      return ++offset;
    }

    if(rcv_ptr == n) {
      rcv_ptr = 0;
    } 
    
  }
}
  
  
