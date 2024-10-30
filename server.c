#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>


void error(const char *error_message) {
    perror(error_message);
    exit(EXIT_FAILURE);
}

