#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

void error(const char *error_message) {
    fprintf(stderr, "%s", error_message);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    



    if (argc < 3) error("Error, expecting: ./server [IP address] [PORT NUMBER] [CLIENT NAME]");
    
    
}