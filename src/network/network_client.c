#include "network/network_client.h"
#include "utils/array_utils.h"
#include "utils/lcg.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <assert.h>

#define DUMMY_LOGO_B64 "iVBORw0KGgoAAAANSUhEUgAAAQAAAAEAAQMAAABmvDolAAAAA1BMVEW10NBjIGi0AAAAH0lEQVRoge3BAQ0AAADCoPdPbQ43oAAAAAAAAAAAvg0hAAABmmDh1QAAAABJRU5ErkJggg=="

static void recv_exact(int sock, uint8_t* buffer, size_t length) {
    ssize_t bytes_received = recv(sock, buffer, length, MSG_WAITALL); // MSG_WAITALL cuz else it might return less bytes
    if (bytes_received != (ssize_t)length) {
        perror("Fatal Network Error: Failed to receive expected bytes");
        exit(EXIT_FAILURE);
    }
}

void network_client_connect(const AgentConfig* config, NetworkClient* out_client) {
    assert(config != NULL);
    assert(config->host != NULL);
    assert(config->agent_name != NULL);

    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP

    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", config->port);

    if (getaddrinfo(config->host, port_str, &hints, &res) != 0) {
        perror("Fatal Error: Could not resolve hostname");
        exit(EXIT_FAILURE);
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        perror("Fatal Error: Could not create socket");
        exit(EXIT_FAILURE);
    }

    printf("Connecting to Server %s:%d...\n", config->host, config->port);

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        perror("Fatal Error: Could not connect to server");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);

    printf("TCP Handshake with Server...\n");

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    uint8_t ping_out = 1;
    if (send(sock, &ping_out, 1, 0) != 1) {
        perror("Fatal Error: Failed to send ping");
        exit(EXIT_FAILURE);
    }

    gettimeofday(&end_time, NULL);
    int latency = (int)(((end_time.tv_sec - start_time.tv_sec) * 1000) +
                        ((end_time.tv_usec - start_time.tv_usec) / 1000));

    uint8_t ping_in;
    recv_exact(sock, &ping_in, 1);
    if (ping_in != 1) {
        fprintf(stderr, "Fatal Error: Outdated client or server version mismatch!\n");
        exit(EXIT_FAILURE);
    }


    dprintf(sock, "%s\n", config->agent_name);
    dprintf(sock, "%s\n", DUMMY_LOGO_B64);

    printf("Sent Name and Image to Server!\n");
    printf("Waiting for Game Config...\n");

    uint8_t server_config;
    recv_exact(sock, &server_config, 1);

    int player_num = server_config & 3;
    int time_limit = server_config / 4;

    // Java sends this Little-Endian!!
    uint8_t seed_bytes[4];
    recv_exact(sock, seed_bytes, 4);

    uint64_t random_seed = (uint64_t)(seed_bytes[0]) |
                          ((uint64_t)(seed_bytes[1]) << 8) |
                          ((uint64_t)(seed_bytes[2]) << 16) |
                          ((uint64_t)(seed_bytes[3]) << 24);

    // Handshake complete!
    // Build the NetworkClient object

    out_client->socket_fd = sock;
    out_client->player_number = player_num;
    out_client->time_limit_sec = time_limit;
    out_client->latency_ms = latency;

    lcg_set_seed(&out_client->rng, (uint64_t)random_seed);

    for (int i = 0; i < 19; i++) {
        out_client->board_heights[i] = (board_height_t)(i + 1);
    }

    board_height_array_shuffle(out_client->board_heights, 19, &out_client->rng);

    printf("--- Connected to Server ---\n");
    printf("Player Number : %d\n", out_client->player_number);
    printf("Time Limit    : %d seconds\n", out_client->time_limit_sec);
    printf("Seed Received : %" PRIu64 "\n", random_seed);
}

void network_client_cleanup(NetworkClient* client) {
    if (client != NULL) {
        if (client->socket_fd >= 0) {
            close(client->socket_fd);
        }
        free(client);
    }
}
