#pragma once

#include <netinet/in.h>
#include "common.h"

/**
 * \brief Creates a TCP socket and connects to a remote server.
 * \param ip_addr  IP address string.
 * \param port     Remote TCP port.
 * \return Returns a socket file descriptor on success, or ERROR on failure.
 */
int connect_to_server(const char *ip_addr, int port);

/**
 * \brief Creates a TCP listening socket and binds it to a port.
 * \param port     Local TCP port to listen on.
 * \param backlog  Max number of pending connections the kernel will queue.
 * \return Returns the listener socket descriptor on success, or ERROR on failure.
 */
int open_connection(int port, int backlog);

/**
 * \brief Accepts an incoming client connection.
 * \param listener_fd The socket returned by open_connection().
 * \return Returns a new socket descriptor for the client, or ERROR on failure.
 */
int accept_client(int listener_fd);