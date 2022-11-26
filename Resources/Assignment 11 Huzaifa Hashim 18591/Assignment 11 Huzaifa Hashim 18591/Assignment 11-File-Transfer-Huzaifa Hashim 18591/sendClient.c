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
#define NUM_THREAD 5
#include <pthread.h>
#define SERVER_PORT 1500

pthread_t threads[NUM_THREAD];
int extraBytes = 0;
pthread_mutex_t s;
int threadNum = 0;
char ip[15];
struct params
{
  char file_name[20];
  char file_format[4];
  int sd;
};

int * createSock()
 {
  int sd, rc, i;
  struct sockaddr_in localAddr, servAddr;
  struct hostent *h;

  h = gethostbyname(ip);
  if(h==NULL) {
    printf("error: unknown host '%s'\n",ip);
    exit(1);
  }

  servAddr.sin_family = h->h_addrtype;
  memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  servAddr.sin_port = htons(SERVER_PORT);

  /* create socket */
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if(sd<0) {
    perror("cannot open socket ");
    exit(1);
  }

  /* bind any port number */
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddr.sin_port = htons(0);
  
  rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));
  if(rc<0) {
    printf("%s: cannot bind port TCP\n",SERVER_PORT);
    perror("error ");
    exit(1);
  }
				
  /* connect to server */
  rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(rc<0) {
    perror("cannot connect ");
    exit(1);
  }



  return (int*)sd;

 } // end of creatSock func()

 int *pieceSize( char file[])
 {
   
   int size;
   int sfd = open(file, O_RDONLY);
   //error opening the file
   if (sfd == -1)
   {
     perror("Error while opening the file.\n");
     exit(EXIT_FAILURE);
   }

   //figure out the file size
   size = lseek(sfd, 0, SEEK_END);
   int chunk = size / NUM_THREAD;
   close(sfd);
   if ((size % NUM_THREAD) != 0)
   {
     
     extraBytes = size % NUM_THREAD;
    
   }
   return ((int *)chunk);
 }



void* chunkin(void* input)
{
	char file[20];
		
	strcpy(file,((struct params*)input)->file_name);
	int sd = (int)createSock();
        int srcc = open(file, O_RDONLY);
       
	 int chunk=(int)pieceSize( file);	
	if (threadNum == 4){
	chunk = chunk + extraBytes;
	//printf("In thread 4. extraBytes is:%d\n", extraBytes);
	}

	void* buf[chunk];
	bzero(buf, chunk);

	//pthread_mutex_lock(&s);
        lseek( srcc, chunk*(threadNum), SEEK_SET ); // set pointer pos
       //increase the thread number here
	threadNum++;
	extraBytes = 0;

	//pthread_mutex_unlock(&s);
 
	int ret = read(srcc,buf,chunk);
	printf("\nthread is writing\n");
	write(sd,buf,ret); // write to socket here	    
	//printf("%s", buf);
	bzero(buf, chunk);
	close(srcc);

	pthread_exit(NULL);

	
}

int main (int argc, char *argv[]) {

  char input[20];
  char inputCpy[20];
  
  
  printf("Enter the IP Address of the server: ");
  scanf(" %s", ip);
  printf("Options:\n 1. a10.txt\n 2. a10.jpg\n 3. a10.rar\n 4. a10.mp4\nEnter the name of the file to send:"); 
  bzero(input, 20);
  scanf(" %s", input);
  getchar();
  printf("File name: %s\n", input);

  // create connection
  int sock = (int)createSock();

  //check if file even exists
  int inputtedFile = open(input, O_RDONLY);
   //error opening the file
   if (inputtedFile == -1)
   {
     perror("Error while opening the file.\n");
     exit(EXIT_FAILURE);
   }
   else
   {
     write(sock,input,sizeof(input));
   }
 
 int i;
 // initialise args
 struct params* args = (struct params*)malloc(sizeof(struct params));
    //args->threadNum = 0;
    strcpy(args->file_name,input);
    

  if (pthread_mutex_init(&s, NULL) !=0) {
     printf("\nmutex init has failed.\n");
  }
  

 // create threads and join them
  for (i = 0; i < NUM_THREAD; i++)
    {
	
      pthread_create(&threads[i], NULL, &chunkin, (void *)args);
	//printf("&threads[i] is %d", (pthread_t)threads[i]);
     
      
    }
  
  for ( i = 0; i < NUM_THREAD; i++)
    {
      pthread_join(threads[i], NULL);
    }
  pthread_mutex_destroy(&s);

  
  printf("\nFinished.");
return 0;
  
}

// Huzaifa Hashim - 18591