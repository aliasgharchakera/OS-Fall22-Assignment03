#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 

#define SERVER_PORT 1500
#define MAX_MSG 100

//Huzaifa Hashim 18591

int main (int argc, char *argv[]) {

  int sd, rc, i;
  struct sockaddr_in localAddr, servAddr;
  struct hostent *h;
  
  
  printf("Enter the IP Address of the server: ");
  char ip[15];
  scanf(" %s", ip);

  h = gethostbyname(ip);
  if(h==NULL) {
    printf("%s: unknown host '%s'\n",argv[0],argv[1]);
    exit(1);
  }

  servAddr.sin_family = h->h_addrtype;
  memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  servAddr.sin_port = htons(SERVER_PORT);

  sd = socket(AF_INET, SOCK_STREAM, 0);
  if(sd<0) {
    perror("cannot open socket ");
    exit(1);
  }

  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddr.sin_port = htons(0);
  
  rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));
  if(rc<0) {
    printf("%s: cannot bind port TCP %u\n",argv[0],SERVER_PORT);
    perror("error ");
    exit(1);
  }
				
  rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(rc<0) {
    perror("cannot connect ");
    exit(1);
  }
  

char data[100000];
char dataCpy[100000];
char s;
int len;
int n;
printf("Press enter to start.");
scanf("%c", &s);
getchar();


int sfd = open("source.txt", O_RDONLY);
if (sfd == -1) {
printf("Can't open source file.");
}
void* buf [20];
while (len = read(sfd, buf, 1) !=0) {
write(STDOUT_FILENO, buf, len);
rc = send(sd, buf, 1, 0);
sleep(1);
bzero(buf, sizeof(buf));
}
printf("\nData sent successfully.\n");
close(sfd);
return 0;
  
}

