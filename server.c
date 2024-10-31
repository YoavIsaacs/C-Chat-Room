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

void error(const char *error_message)
{
    fprintf(stderr, "%s", error_message);
    fflush(stdout);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        error("Port number not provided. Terminating program.\n");
    }
    else if (argc > 2)
    {
        error("Too many arguments. Expectimg [server] [port number]. Please try again, terminating.\n");
    }

    int sockfd;
    int newsocfd;
    int fd_max;
    fd_set master_set;
    fd_set read_fds;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_address;
    socklen_t client_len;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("Error opening socket.\n");
    }

        // Set up the server address
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(atoi(argv[1]));

    // Bind the socket to the specified port
    if (bind(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        error("Error binding socket.\n");
    }

    // Start listening for connections
    if (listen(sockfd, NUMBER_OF_POSSIBLE_CLIENTS_THAT_CAN_CONNECT) < 0)
    {
        error("Error listening on socket.\n");
    }

    FD_ZERO(&master_set);
    FD_ZERO(&read_fds);

    FD_SET(sockfd, &master_set);
    fd_max = sockfd;

    printf("Server running on port %s", PORT_NUMBBER);
    fflush(stdout);

    while (ALWAYS)
    {

        read_fds = master_set;
        if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1)
            error("Error with select, terminating.\n");

        for (int i = 0; i <= fd_max; i++)
        {
            if (FD_ISSET(i, &read_fds))
            { // We found a valid connection
                if (i == sockfd)
                { // New connection
                    client_len = sizeof(client_len);
                    newsocfd = accept(sockfd, (struct sockaddr *)&server_address, &client_len);
                    if (newsocfd < 0)
                        error("Error accepting new connection, terminating.\n");
                    else
                    {
                        FD_SET(newsocfd, &master_set); // Add new connection FD to master set.
                        if (newsocfd > fd_max)
                            fd_max = newsocfd;
                        printf("New connection from %s on socket %d\n", inet_ntoa(server_address.sin_addr), newsocfd);
                        fflush(stdout);
                    }
                }
            }
            else
            { // Handle data gotten from a client
                bzero(buffer, BUFFER_SIZE);
                int nbytes = read(i, buffer, sizeof(buffer));
                if (nbytes <= 0)
                { // Connection closed.
                    if (nbytes == 0) {
                        printf("Socket %d closed.\n", i);
                        fflush(stdout);
                    } else
                        perror("recv");
                    close(i);
                    FD_CLR(i, &master_set); // Remove socket from master set
                }
                else
                { // Handle data gotten from client by broadcasting it to all other clients
                    for (int j = 0; j <= fd_max; j++)
                    {
                        // Broadcast to all except for the listener and itself
                        if (FD_ISSET(j, &master_set))
                        {
                            if (j != sockfd && j != i)
                            {
                                if (write(j, buffer, nbytes) < 0)
                                    perror("write");
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
}