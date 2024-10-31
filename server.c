#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define PORT_NUMBBER argv[1]
#define NUMBER_OF_POSSIBLE_CLIENTS_THAT_CAN_CONNECT 5
#define ALWAYS 1
#define BUFFER_SIZE 1024


void error(const char *error_message) {
    fprintf(stderr, "%s", error_message);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        error("Port number not provided. Terminating program.\n");
    } else if (argc > 2) {
        error("Too many arguments. Expectimg [server] [port number]. Please try again, terminating.\n");
    }

    int sockfd;
    int newsocfd;
    int port_number;
    int check;
    int exit_check;

    char buffer[BUFFER_SIZE];

    struct sockaddr_in server_addr, cli_addr;

    socklen_t client_len;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        error("Error opening socket.\n");
    }

    bzero((char *) &server_addr, sizeof(server_addr));

    port_number = atoi(PORT_NUMBBER);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_number);

    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        error("Error binding socket, terminating.\n");


    listen(sockfd, NUMBER_OF_POSSIBLE_CLIENTS_THAT_CAN_CONNECT);
    client_len = sizeof(cli_addr);

    newsocfd = accept(sockfd, (struct sockaddr *)&cli_addr, &client_len);
    if (newsocfd < 0) {
        error("Error accpeing, terminating.\n");
    }

    while (ALWAYS) {
        bzero(buffer, BUFFER_SIZE);
        check = read(newsocfd, buffer, BUFFER_SIZE);
        if (check < 0) {
            error("Error reading from client, terminating.\n");
        }
        printf("Client: %s\n", buffer);
        bzero(buffer, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);

        check = write(newsocfd, buffer, strlen(buffer));
        if (check < 0) {
            error("Error sending message, terminating.\n");
        }

        exit_check = strncmp("Bye", buffer, strlen("Bye"));
        if (exit_check == 0) break;
    }

    close(newsocfd);
    close(sockfd);
    exit(EXIT_SUCCESS);
}