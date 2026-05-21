#include "core/agent_core.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    AgentContext ctx;

    agent_init(&ctx, argc, argv, "Idler", "Play illegal move and break immediately");

    // Custom loop for idler
    while (game_network_receive_move(&ctx.game_settings, &ctx.client, &ctx.game)) {
        // Just consume moves until it's our turn
    }

    uint8_t illegal_move = ILLEGAL_MOVE;
    printf("Sending illegal move %u\n", illegal_move);
    game_network_send_move(&ctx.game_settings, &ctx.client, illegal_move);

    printf("Exiting now!\n");

    return EXIT_SUCCESS;
}
