#include "tcp_lib.h"
#include "common.h"

#include <unistd.h>

#define SERVER_IP "127.0.0.1"

int main(void) {
    int sockfd;
    char input[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    ssize_t bytes_received;

    printf("Starting client...\n");

    sockfd = connect_to_server(SERVER_IP, SERVER_PORT);
    if (sockfd == -1) {
        fprintf(stderr, "Failed to connect to server.\n");
        return ERROR;
    }

    printf("Connected to server.\n");

    while (1) {
        printf("SQL> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) {
            break; // EOF
        }

        // Send cmd to server
        send(sockfd, input, strlen(input), 0);

        if (strncmp(input, "EXIT", 4) == 0) {
            printf("Closing connection...\n");
            break;
        }

        // Receive length-prefixed response: first read 4-byte length
        uint32_t net_len;
        bytes_received = recv(sockfd, &net_len, sizeof(net_len), 0);
        if (bytes_received <= 0) {
            printf("Server closed connection.\n");
            break;
        }
        if (bytes_received != sizeof(net_len)) {
            // try to read remaining bytes of the length header
            size_t need = sizeof(net_len) - bytes_received;
            char *write_ptr = ((char *)&net_len) + bytes_received;
            while (need > 0) {
                ssize_t more_bytes = recv(sockfd, write_ptr, need, 0);
                if (more_bytes <= 0) {
                    break;
                }
                need -= more_bytes;
                write_ptr += more_bytes;
            }
        }

        uint32_t payload_len = ntohl(net_len);
        if (payload_len == 0) {
            printf("(no data)\n");
            continue;
        }

        size_t to_read = (payload_len < sizeof(response) - 1)
            ? payload_len
            : (sizeof(response) - 1);
        size_t received = 0;
        while (received < to_read) {
            bytes_received = recv(sockfd, response + received, to_read - received, 0);
            if (bytes_received <= 0)
                break;
            received += bytes_received;
        }
        response[received] = '\0';
        printf("%s\n", response);

        // If server payload was larger than our buffer, delete the remainder
        if (payload_len > to_read) {
            size_t remaining = payload_len - to_read;
            char discard[512];
            while (remaining > 0) {
                bytes_received = recv(sockfd, discard,
                    (remaining > sizeof(discard)) ? sizeof(discard) : remaining,
                    0);
                if (bytes_received <= 0)
                    break;
                remaining -= bytes_received;
            }
        }
    }

    close(sockfd);
    return SUCCESS;
}
