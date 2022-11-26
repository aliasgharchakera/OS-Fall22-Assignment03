#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

struct file_params{
    char *ip_address;
    int transfer_port;
    int start_index;
    int offset;
    char filename[1024];
};


void send_chunk(int server_socket, struct file_params* ptr){

    FILE *fp = fopen(ptr->filename, "r");
    printf("Sending data. start_index: %d, offset: %d\n", ptr->start_index, ptr->offset);
    int n;
    char data[1] = {0};
    int count = 0;

    fseek(fp, ptr->start_index, SEEK_SET);
    while (((n = fread(data, sizeof(char), 1, fp)) > 0) && (count <= ptr->offset)){
        
        if (send(server_socket, data, n, 0) < 0){
            perror("Error: Error in sending file.");
            exit(1);
        }
    
        bzero(data, 1);
        count++;
    }
}

void *thread(void* p){

    struct file_params* ptr = (struct file_params*) p;
    int server_socket, client_socket, n;
    struct sockaddr_in server_addr, client_address;
    socklen_t address_size;

    printf("[/] Socket port: %d\n", ptr->transfer_port);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0){
        perror("Error: Error in socket.\n");
        exit(1);
    }

    printf("Server socket created successfully.\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = ptr->transfer_port;
    server_addr.sin_addr.s_addr = inet_addr(ptr->ip_address);
    n = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    if (n < 0){
        exit(1);
    }

    printf("Binding successfull.\n");
    if (listen(server_socket, 10) == 0){
        printf("Listening....\n");
    }

    else{
        exit(1);
    }

    address_size = sizeof(client_address);
    client_socket = accept(server_socket, (struct sockaddr*)&client_address, &address_size);
    printf("Client connected!\n");
    send_chunk(client_socket, ptr);
    printf("Data sent to the Client successfully.\n");
    close(client_socket);
    return p;
}

int main(int argc){
    char *ip = "127.0.0.1";
    int port=5566;
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t address_size;
    char buffer[1024];
    int n;
    FILE *fp;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    if (server_socket < 0){
        perror("[-]Socket error");
        exit(1);
    }
	
    printf("[+]Server socket created successfully.\n");
    memset(&server_address,'\0',sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = port;
    server_address.sin_addr.s_addr = inet_addr(ip);

    n = bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    
    if (n < 0){
        perror("[-]Binding Error");
        exit(1);
    }
    printf("[+]Binding successfull.\n");
    listen(server_socket, 5);
    printf(" Listening....\n");

    while (1){
        address_size = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &address_size);
        printf("[+]Client connected.\n");

        bzero(buffer, 1024);
        recv(client_socket, buffer, 1024, 0);
        printf("[+]Filename received: %s \n", buffer);

        strcat(buffer, '\0');
        char filename[1024];
        strcat(filename, buffer);
        fp = fopen(buffer, "rb");

        int size = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        printf("File Size: %d", size);

        int n_sessions = 1;
        bzero(buffer, 1024);
        
        n_sessions = ntohl(n_sessions);
        printf("The number of threads requested is: %d\n", n_sessions);

        int new_size = htonl(size);
        
        printf("File size Sent: %d\n", size);

        pthread_t tid[n_sessions];
        struct file_params params[n_sessions];
        int offset = (size / n_sessions);

        for (int i = 0; i < n_sessions; i++){
            params[i].start_index = i*offset;
            params[i].ip_address = ip;
            params[i].transfer_port = port + i*5 + 1;
            strcpy(params[i].filename,"");
            strcat(params[i].filename, filename);
            printf("Calling thread %d with port %d.\n", i, params[i].transfer_port);

            if (i == n_sessions - 1){
                params[i].offset = size - offset*(n_sessions);
            }
            
            else{
                params[i].offset = offset;
            }

            pthread_create(&tid[i], NULL, thread, &params[i]);
        }
        
        for (int i = 0; i < n_sessions; i++){
            pthread_join(tid[i], NULL);
        }

        bzero(buffer, 1024);
        close(client_socket);
        close(server_socket);
        fclose(fp);
        break;
    }
    return 0;
}