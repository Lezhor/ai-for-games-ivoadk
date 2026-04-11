#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "cli.h"
#include "network/network_client.h"

int main(int argc, char *argv[]) {
    AgentConfig config = parse_args(argc, argv);

    printf("--- IVOADK Idler Client ---\n");
    printf("Target Host: %s\n", config.host);
    printf("Target Port: %d\n", config.port);
    printf("Agent Name: %s\n", config.agent_name);
    printf("Strategy: Play illegal move and break immediately\n");
    printf("----------------------------\n");

    NetworkClient* client = calloc(1, sizeof(NetworkClient));
    assert(client != NULL && "Failed to allocate NetworkClient");

    network_client_connect(&config, client);

    Move move;
    while (network_client_receive_move(client, &move)) {
        printf("Received move %u from player %u\n", move.index, move.player);
    }
    move.index = 19;
    printf("Sending illegal move %u\n", move.index);
    network_client_send_move(client, move.index);

    printf("Exiting now!");

    return EXIT_SUCCESS;
}
