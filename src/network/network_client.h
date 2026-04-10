#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include "board.h"
#include "utils/lcg.h"

typedef struct AgentConfig {
    const char* host;
    int port;
    const char* agent_name;
} AgentConfig;

typedef struct NetworkClient {
    int socket_fd; // POSIX file descriptor for the TCP socket

    // provided by server
    lcg_t rng; // NOTE: we might not need it after board init? idk.
    int player_number;
    int time_limit_sec;
    int latency_ms;

    board_height_t board_heights[19];
} NetworkClient;

/**
 * Initial Handshake with jar server.
 * 1. receive 0x01 from Server
 * 2. ... // TODO: complete Protocol Description
 */
NetworkClient* network_client_connect(const AgentConfig* config);

/**
 * Closes connection and cleans socket
 */
void network_client_cleanup(NetworkClient* client);


// -----------------------------------------
// API (send, receive)
// -----------------------------------------

// TODO: send and receive API endpoints

#endif
