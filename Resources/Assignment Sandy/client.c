#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

struct file_params{
    int offset;
    int start_index;
    char file_id[3];
    FILE *file_to_receive;
    char *fname;
    char *ip_address;
    char *file_data;
    int transfer_port;
    
};

void *file_write(int sock, char *duplicate, struct file_params* ptr){

    int n = 0;
    FILE *fp;
    char filename[1024] = "";
    strcat(filename, ptr->file_id);
    strcat(filename, duplicate);
    char buffer[1];
    bzero(buffer, 1);

    fp = fopen(filename, "w");
    while (1){
        n = recv(sock, buffer, 1, 0);
        
        if (n <= 0){
            break;
        }

        int write_size = fwrite(buffer, sizeof(char), n, fp);
        
        if(write_size < n){
            perror("[-] File write failed on your pc.\n");
            exit(1);
        }   
        bzero(buffer, 1);
    }
    fclose(fp);
    return ptr;
}

void *thread(void* p){

    struct file_params* ptr = (struct file_params*) p;
    int sock, n;
    struct sockaddr_in address;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    if(sock < 0){
        perror("[-] Error in socket");
        exit(1);
    }

    printf("*** Server socket created successfully.\n");
    address.sin_family = AF_INET;
    address.sin_port = ptr->transfer_port;
    address.sin_addr.s_addr = inet_addr(ptr->ip_address);

    printf("[/] Establishing connection with socket port: %d\n", ptr->transfer_port);
    sleep(2);
    n = connect(sock, (struct sockaddr*)&address, sizeof(address));
    
    if(n == -1){
        perror("[-] Error in socket");
        exit(1);
    }

    file_write(sock, ptr->fname, ptr);
    printf("File data sent successfully on port: %d.\n", ptr->transfer_port);
    printf("Closing the connection.\n");
    close(sock);
    return p;
}

int main(int argc, char **argv){
    char *ip = "127.0.0.1";
    int port=5566;
    int sock;
    struct sockaddr_in address;
    socklen_t address_size;
    char buffer[1024];
    int n;
    FILE *fp;
    char filename[64];
    printf("Enter the filename:");
    scanf("%s", filename);
    int total_threads;
    printf("Enter the number of threads:");
    scanf("%d", &total_threads);

    pthread_t tid[total_threads];
    struct file_params params[total_threads];
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    if(sock < 0){
        perror("Error: Error in socket");
        exit(1);
    }
    printf("[+]Server socket created successfully.\n");
    memset(&address,'\0',sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = port;
    address.sin_addr.s_addr = inet_addr(ip);
    
    connect(sock, (struct sockaddr*)&address, sizeof(address));
    printf("[+]Connected to the server.\n");

    int sessions = htonl(total_threads);

    int filesize = 0;
    
    filesize = ntohl(filesize);
    printf("File size received: %d\n", filesize);

    int offset = (filesize / total_threads);

    for (int i = 0; i < total_threads; i++){
        params[i].file_to_receive = fp;
        params[i].fname = filename;

        sprintf(params[i].file_id, "%d", i);
        params[i].ip_address = ip;
        params[i].transfer_port = port + i*5 + 1;
        
        
        if (i == total_threads - 1){
            params[i].offset = filesize - offset*(total_threads);
        }

        else{
            params[i].offset = offset;
        }

        params[i].start_index = offset*i;
        params[i].file_data = (char *) malloc(sizeof(char)*offset);
        printf("Calling thread %d with port %d.\n", i, params[i].transfer_port);
        pthread_create(&tid[i], NULL, thread, &params[i]);
    }

    for (int i = 0; i < total_threads; i++){
        pthread_join(tid[i], NULL);
    }

    int last_position = 0;
    char final[1024] = "received_";
    strcat(final, filename);
    FILE *file_received = fopen(final, "a");

    for (int i = 0; i < total_threads; i++){
        char name[1024];
        sprintf(name, "%d", i);
        strcat(name, filename);

        FILE *handle = fopen(name, "r");
        char arr[offset];
        fread(arr, 1, offset, handle);
        fseek(file_received, last_position, SEEK_SET);
        fwrite(arr, 1, offset, file_received);
        last_position = i * offset;
        fclose(handle);
        remove(name);
    }

    fclose(file_received);
    printf("Closing the connection.\n");
    close(sock);

    for (int i = 0; i < total_threads; i++){
        free(params[i].file_data);
    }

    return 0;
}