#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFER_SIZE 1024
#define MAX_NAME_LENGTH 256
#define ALWAYS 1

// CHABE ALL bzero() TO MEMSER WITH 0 LATER

void error(const char *error_message) {
    fprintf(stderr, "%s", error_message);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    
    int sockdf;
    int port_number;
    int check;
    struct sockaddr_in server_address;
    struct hostent *server;
    char buffer[BUFFER_SIZE];
    char client_name[MAX_NAME_LENGTH];
    char message[BUFFER_SIZE];

    if (argc < 3) error("Error, expecting: ./server [IP address] [PORT NUMBER] [CLIENT NAME]\nTerminating.\n");
    
    port_number = atoi(argv[2]);
    strcpy(client_name, argv[3]);
    strcat(client_name, ": ");
    sockdf = socket(AF_INET, SOCK_STREAM, 0);
    if (sockdf < 0 ) error("Error opening socket, terminating.\n");
    
    server = gethostbyname(argv[1]);
    if (server == NULL) error("Error, invalid server IP address or the server is down, terminating.\n");

    bzero((char *)&server_address, sizeof(server_address));
    

    server_address.sin_family = AF_INET;
    bcopy((char *) server->h_addr_list[0], (char *) &server_address.sin_addr, server->h_length);

    server_address.sin_port = htons(port_number);
    if (connect(sockdf, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) error("Error connecting to server, terminating.\n");

    while (ALWAYS) {
        bzero(buffer, BUFFER_SIZE);
        bzero(message, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);
        strcat(message, client_name);
        strcat(message, buffer);
        check = write(sockdf, message, sizeof(message));
        if (check < 0) error("Error sending message to server, terminating.\n");

        bzero(buffer, BUFFER_SIZE);
        bzero(message, BUFFER_SIZE);

        check = read(sockdf, buffer, BUFFER_SIZE);
        if (check < 0) error("Error recieving message from server, terminating.\n");
        printf("Server: %s\n", buffer);

        check = strncmp("Bye", buffer, sizeof("Bye"));
        if (check == 1) break;
    }

    close(sockdf);
    exit(EXIT_SUCCESS);

    

}