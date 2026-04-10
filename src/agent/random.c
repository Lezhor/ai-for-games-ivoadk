#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "cli.h"
#include "network/network_client.h"

/**
 * minimal test
 * Usage: ./agent <host> <port>
 */
int main(int argc, char *argv[]) {
    AgentConfig config = parse_args(argc, argv);

    printf("--- IVOADK Random Client ---\n");
    printf("Target Host: %s\n", config.host);
    printf("Target Port: %d\n", config.port);
    printf("Agent Name: %s\n", config.agent_name);

    NetworkClient* client = calloc(1, sizeof(NetworkClient));
    assert(client != NULL && "Failed to allocate NetworkClient");

    network_client_connect(&config, client);

    printf("Game Started as player %d!", client->player_number);

    return EXIT_SUCCESS;
}
