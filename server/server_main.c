#include "common.h"
#include "sql_executor.h"
#include "sql_parser.h"
#include "tcp_lib.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BACKLOG 10

int main(void) {
    int listener_fd, client_fd;

    printf("Starting server...\n");

    listener_fd = open_connection(SERVER_PORT, BACKLOG);
    if (listener_fd == ERROR) {
        fprintf(stderr, "Failed to open listener socket.\n");
        return ERROR;
    }

    printf("Listening on port %d...\n", SERVER_PORT);

    client_fd = accept_client(listener_fd);
    if (client_fd == ERROR) {
        fprintf(stderr, "Failed to accept client.\n");
        close(listener_fd);
        return ERROR;
    }

    printf("Client connected.\n");

    while (1) {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        buffer[bytes_received] = '\0';
        printf("Received: %s\n", buffer);

        // Cmd EXIT
        if (strncmp(buffer, "EXIT", 4) == 0) {
            printf("Client requested exit.\n");
            break;
        }

        struct sql_query q;
        char response[20480];
        memset(response, 0, sizeof(response));

        // Try to parse sql command
        if (parse_sql(buffer, &q) != 0) {
            snprintf(response, sizeof(response), "ERROR: Invalid SQL\n");
            send(client_fd, response, strlen(response), 0);
            continue;
        }

        // Execute
        switch (q.type) {
        case SQL_SELECT:
            execute_select_all(&q, response, sizeof(response));
            break;

        case SQL_INSERT:
            execute_insert(&q.data.insert_q, response, sizeof(response));
            break;

        case SQL_UPDATE:
            execute_update(&q.data.update_q, response, sizeof(response));
            break;

        case SQL_DELETE:
            execute_delete(&q.data.delete_q, response, sizeof(response));
            break;

        default:
            snprintf(response, sizeof(response), "ERROR: Unsupported SQL\n");
            break;
        }

        // Send a 4-byte length prefix (network byte order), then the payload
        uint32_t payload_len = (uint32_t)strlen(response);
        uint32_t net_len = htonl(payload_len);
        send(client_fd, &net_len, sizeof(net_len), 0);
        if (payload_len > 0) {
            send(client_fd, response, payload_len, 0);
        }
    }

    close(client_fd);
    close(listener_fd);

    printf("Server shutdown.\n");
    return SUCCESS;
}
