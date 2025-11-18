#include "tcp_lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BACKLOG 10

int main(void) {
    int listener_fd, client_fd;

    printf("Starting server...\n");

    // 1. Open server socket
    listener_fd = open_connection(SERVER_PORT, BACKLOG);
    if (listener_fd == ERROR) {
        fprintf(stderr, "Failed to open listener socket.\n");
        return ERROR;
    }

    printf("Listening on port %d...\n", SERVER_PORT);

    // 2. Accept one client (blocking) **
    client_fd = accept_client(listener_fd);
    if (client_fd == ERROR) {
        fprintf(stderr, "Failed to accept client.\n");
        close(listener_fd);
        return ERROR;
    }

    printf("Client connected.\n");

    // 3. Read a line from client
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        fprintf(stderr, "Error receiving data or client disconnected.\n");
    } else {
        buffer[bytes_received] = '\0';
        printf("Received from client: %s", buffer);

        // 4. Send response to client
        const char *response = "Hello from server\n";
        ssize_t bytes_sent = send(client_fd, response, strlen(response), 0);
        if (bytes_sent == -1) {
            perror("Error sending response to client");
        } else {
            printf("Sent to client: %s", response);
        }
    }
    close(client_fd);
    close(listener_fd);

    printf("Shutdown.\n");
    return SUCCESS;
}
