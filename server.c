#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>

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
    int fd_max;
    fd_set master_set;
    fd_set read_fds;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in cli_addr;
    socklen_t client_len;


    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        error("Error opening socket.\n");
    }


    FD_ZERO(&master_set);
    FD_ZERO(&read_fds);

    FD_SET(sockfd, &master_set);
    fd_max = sockfd;

    while (ALWAYS) {

        read_fds = master_set;
        if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1) error("Error with select, terminating.\n");
        
        for (int i = 0; i <= fd_max; i++) {
            if (FD_ISSET(i, &read_fds)) { // We found a valid connection
                if (i == sockfd) { //New connection
                    client_len = sizeof(cli_addr);
                    newsocfd = accept(sockfd, (struct sockaddr *)&cli_addr, &client_len);
                    if (newsocfd < 0) error("Error accepting new connection, terminating.\n");
                    else {
                        FD_SET(newsocfd, &master_set); // Add new connection FD to master set.
                        if (newsocfd > fd_max) fd_max = newsocfd;
                        printf("New connection from %s on socket %d\n", inet_ntoa(cli_addr.sin_addr), newsocfd);
                    }
                }
            } else { // Handle data gotten from a client
                bzero(buffer, BUFFER_SIZE);
                int nbytes = read(i, buffer, sizeof(buffer));
                if (nbytes <= 0) { //Connection closed.
                    if (nbytes == 0) printf("Socket %d closed.\n", i);
                else perror("recv");
                close(i);
                FD_CLR(i, &master_set); // Remove socket from master set
            } else { // Handle data gotten from client by broadcasting it to all other clients
                for (int j = 0; j <= fd_max; j++) {
                    // Broadcast to all except for the listener and itself
                    if (FD_ISSET(j, &master_set)) {
                        if (j != sockfd && j != i) {
                            if (write(j, buffer, nbytes) < 0) perror("write");
                        }
                    }
                }
            }
        }
    }
    }

    close(newsocfd);
    close(sockfd);
    exit(EXIT_SUCCESS);
    /*
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
        printf("%s\n", buffer);
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
*/
}