
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
void error(const char*msg){

perror(msg);
exit(1);
}
int main (int argc, char *argv[]) {

  int sd, rc, i, n;
  struct sockaddr_in localAddr, servAddr;
  struct hostent *h;
  

  char buffer[100000];
  printf("Type IP Address of the Server: ");
  char ip[15];
  scanf(" %s", ip);
getchar();
  h = gethostbyname(ip);
  if(h==NULL) {
    printf("%s: host not available '%s'\n",argv[0],argv[1]);
    exit(1);
  }

  servAddr.sin_family = h->h_addrtype;
  memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  servAddr.sin_port = htons(SERVER_PORT);

  /* creating socket */
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if(sd<0) {
    perror("cannot open socket ");
    exit(1);
  }

  /* binding any port number */
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddr.sin_port = htons(0);
  
  rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));
  if(rc<0) {
    printf("%s: TCP Port cannot be bind %u\n",argv[0],SERVER_PORT);
    perror("error ");
    exit(1);
  }
				
  /* connecting to the server */
  rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(rc<0) {
    perror("Can't connect");
    exit(1);
  }

  int term = 0;
printf("Type your Username and Password (with space) and click the enter twice: ");
bzero(buffer, 100000);
fgets(buffer, 100000, stdin); 
buffer[strcspn(buffer, "\n")] = 0;
n = write(sd, buffer, strlen(buffer));
if (n<0) error("Typing error");
  while (1)
  {
   

    bzero(buffer, 100000);
    fgets(buffer, 100000, stdin); 
     buffer[strcspn(buffer, "\n")] = 0;
    n = write(sd, buffer, strlen(buffer));
    if (n<0) error("Writing error");
    bzero(buffer, 100000);
    n = read(sd, buffer, 100000);
    if (n <0) error("Reading error");
    printf("Server: %s", buffer);
    int i = strncmp("Client closed the connection.\n", buffer, 30);
    if (i ==0) break; 
    int j = strncmp("Closing client connection, incorrect credentials.\n", buffer, 48);
    if (j ==0) break; 
int k = strncmp("Invalid argument, closing client connection\n", buffer, 44);
if (k ==0) break; 
} 
close(sd);
return 0;
//Huzaifa Hashim 18591
}

