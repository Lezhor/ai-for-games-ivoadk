#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "cli.h"
#include "network/network_client.h"
#include "utils/lcg.h"

int main(int argc, char *argv[]) {
    AgentConfig config = parse_args(argc, argv);

    printf("--- IVOADK Randomizer Client ---\n");
    printf("Target Host: %s\n", config.host);
    printf("Target Port: %d\n", config.port);
    printf("Agent Name: %s\n", config.agent_name);
    printf("Strategy: Play random legal moves\n");
    printf("----------------------------\n");

    NetworkClient* client = calloc(1, sizeof(NetworkClient));
    assert(client != NULL && "Failed to allocate NetworkClient");

    network_client_connect(&config, client);

    lcg_t rng;
    lcg_set_seed(&rng, (uint64_t)(lcg_next_int(&client->rng) + client->player_number));

    // game loop

    Move move;
    while (1) {
        while (network_client_receive_move(client, &move)) {
            printf("Received move %u from player %u\n", move.index, move.player);
            // TODO: update board
        }
        // send move
        move.index = (uint8_t)lcg_next_int_n(&rng, 19);
        printf("Sending move %u\n", move.index);
        network_client_send_move(client, move.index);
    }

    printf("Game Started as player %d!", client->player_number);

    return EXIT_SUCCESS;
}
