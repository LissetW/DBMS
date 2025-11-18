#include "tcp_lib.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Steps to create and connect a TCP socket to a remote server - client side
int connect_to_server(const char *ip_addr, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    if (ip_addr == NULL)
        return ERROR;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Creation of socket failed");
        return ERROR;
    }

    // Initialize structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_port = htons(port); // big-endian port

    // Convert IP address string to binary
    if (inet_pton(AF_INET, ip_addr, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton: invalid IP address format\n");
        close(sockfd);
        return ERROR;
    }

    // Attempt connection
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))
        == -1) {
        perror("Connection to server failed");
        close(sockfd);
        return ERROR;
    }

    return sockfd;
}

// Steps to create, bind, and listen on a TCP socket - server side
int open_connection(int port, int backlog) {
    int sockfd;
    struct sockaddr_in my_addr;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Creation of socket failed");
        return ERROR;
    }

    // Enable immediate port reuse
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))
        < 0) {
        perror("Setting socket options failed");
        close(sockfd);
        return ERROR;
    }

    // Fill structure
    my_addr.sin_family = AF_INET; // IPv4
    my_addr.sin_port = htons(port); // big-endian port
    my_addr.sin_addr.s_addr = INADDR_ANY; // listen to all interfaces
    memset(&(my_addr.sin_zero), 0, 8);

    // Bind socket to local port
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))
        == -1) {
        perror("Binding socket to local port failed");
        close(sockfd);
        return ERROR;
    }

    // Start listening
    if (listen(sockfd, backlog) == -1) {
        perror("Starting to listen failed");
        close(sockfd);
        return ERROR;
    }

    return sockfd;
}

// Accept an incoming client connection - server side
int accept_client(int listener_fd) {
    int newfd;
    struct sockaddr_in client_addr;
    socklen_t sin_size = sizeof(struct sockaddr_in);

    if ((newfd
            = accept(listener_fd, (struct sockaddr *)&client_addr, &sin_size))
        == -1) {
        perror("Accepting client connection failed");
        return ERROR;
    }

    return newfd;
}
