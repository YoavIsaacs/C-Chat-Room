#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>


#define BUFFER_SIZE 1024
#define MAX_NAME_LENGTH 256
#define ALWAYS 1


void error(const char *error_message) {
    fprintf(stderr, "%s", error_message);
    fflush(stdout);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    
    int sockdf;
    int port_number;
    int check;
    int fd_max;
    int flags;
    struct sockaddr_in server_address;
    struct hostent *server;
    char buffer[BUFFER_SIZE];
    char client_name[MAX_NAME_LENGTH];
    char message[BUFFER_SIZE];
    fd_set master_set;
    fd_set read_fds;

    if (argc < 4) error("Error, expecting: ./client [IP address] [PORT NUMBER] [CLIENT NAME]\nTerminating.\n");
    
    port_number = atoi(argv[2]);

    // Client name setup
    strncpy(client_name, argv[3], MAX_NAME_LENGTH - 3); // Reserving space for ": "
    client_name[MAX_NAME_LENGTH - 3] = '\0';
    strcat(client_name, ": ");

    // Socket setup
    sockdf = socket(AF_INET, SOCK_STREAM, 0);
    if (sockdf < 0 ) error("Error opening socket, terminating.\n");
    
    // Host connection
    server = gethostbyname(argv[1]);
    if (server == NULL) error("Error, invalid server IP address or the server is down, terminating.\n");


    bzero((char *)&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    bcopy((char *) server->h_addr_list[0], (char *) &server_address.sin_addr, server->h_length);
    server_address.sin_port = htons(port_number);
    if (connect(sockdf, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) error("Error connecting to server, terminating.\n");

    // Setting stdin to non-blocking
    flags = fcntl(STDERR_FILENO, F_GETFL, 0);
    if (flags == -1) error("fcntly F_GETFL");
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1) error("fcntl F_SETFL");

    FD_ZERO(&master_set);
    FD_ZERO(&read_fds);

    FD_SET(STDIN_FILENO, &master_set);
    FD_SET(sockdf, &master_set);
    fd_max = sockdf;

    printf("Connected to server, you can now start sending messages.\n");
    fflush(stdout);



    while (ALWAYS) {
        read_fds = master_set;
        if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1) error("Error with select, ternimating\n");

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            //Handle user input
            bzero(buffer, BUFFER_SIZE);
            if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
                size_t len = strlen(buffer);
                if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';

                bzero(message, BUFFER_SIZE);
                strncpy(message, client_name, MAX_NAME_LENGTH);
                strncat(message, buffer, BUFFER_SIZE - strlen(client_name) - 1);

                check = write(sockdf, message, strlen(message));
                if (check < 0) error("Error sending message to server.\n");

                if (strncmp("exit chat room", buffer, strlen("exit chat room") == 0)) {
                    printf("Exiting chat.\n");
                    fflush(stdout);
                    break;
                }
            } else {
                // EOF reached
                printf("EOF on stdin, exiting.\n");
                fflush(stdout);
                break;
            }
        }

        if (FD_ISSET(sockdf, &read_fds)) {
            // If there is data from the server
            bzero(buffer, BUFFER_SIZE);
            check = read(sockdf, buffer, BUFFER_SIZE - 3);

            if (check <= 0) {
                if (check == 0) printf("The server has closed the connection.\n");
                else error("Error receiving data from the server.\n");
            } else {
                buffer[check] = '\0';
                printf("%s\n", buffer);
                fflush(stdout);
            }
        } 
    }

    if (fcntl(STDIN_FILENO, F_SETFL, flags) == -1) error("fcntl F_SETFL reset");

    close(sockdf);
    exit(EXIT_SUCCESS);
}