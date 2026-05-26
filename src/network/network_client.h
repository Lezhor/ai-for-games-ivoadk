#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include <stdint.h>
typedef struct AgentConfig {
    const char* host;
    int port;
    const char* agent_name;
    const char* icon_path;
} AgentConfig;

typedef struct NetworkClient {
    int socket_fd; // POSIX file descriptor for the TCP socket

    // provided by server
    int32_t seed;
    int player_number;
    int time_limit_sec;
    int latency_ms;

    // local state
    uint64_t input_request_timestamp; // ms
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
 * 4. Client sends Base64 encoded 256x256 PNG icon followed by '\n'.
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
