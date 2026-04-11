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

typedef struct Move {
    uint8_t player;
    uint8_t index;
} Move;

/**
 * Connects to the IVOADK server / initial handshake protocol and game init.
 * * Protocol Flow:
 * 1. Client sends 0x01 (Ping).
 * 2. Server replies with 0x01 (If different, version mismatch/crash).
 * 3. Client sends Team Name string followed by '\n'.
 * 4. Client sends Base64 encoded 256x256 PNG Logo followed by '\n'.
 * 5. Server replies with 1 byte (Config).
 * - Lowest 2 bits = Player Number.
 * - Remaining bits = Time Limit in seconds.
 * 6. Server replies with 4 bytes (Random Seed in Little-Endian order).
 * - using the seed the client shuffles the board.
 */
void network_client_connect(const AgentConfig* config, NetworkClient* out_client);

int network_client_receive_move(NetworkClient* client, Move* out_move);
void network_client_send_move(NetworkClient* client, uint8_t move);

/**
 * Closes connection and cleans socket
 */
void network_client_cleanup(NetworkClient* client);


// -----------------------------------------
// API (send, receive)
// -----------------------------------------

// TODO: send and receive API endpoints

#endif
