#include "tcp_lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"

int main(void) {
    int sockfd;

    printf("Starting client...\n");

    // 1. Connect to server
    sockfd = connect_to_server(SERVER_IP, SERVER_PORT);
    if (sockfd == -1) {
        fprintf(stderr, "Failed to connect to server.\n");
        return ERROR;
    }

    printf("Connected to server at %s:%d\n", SERVER_IP, SERVER_PORT);

    // 2. Send text to server
    const char *msg = "Hello from client\n";
    ssize_t bytes_sent = send(sockfd, msg, strlen(msg), 0);
    if (bytes_sent == -1) {
        fprintf(stderr, "Error sending data to server.\n");
    } else {
        printf("Sent to server: %s", msg);
    }

    // 3. Receive response from server
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        fprintf(stderr, "Error receiving response or server disconnected.\n");
    } else {
        buffer[bytes_received] = '\0';
        printf("Received from server: %s", buffer);
    }
    close(sockfd);

    printf("Disconnected.\n");
    return EXIT_SUCCESS;
}
