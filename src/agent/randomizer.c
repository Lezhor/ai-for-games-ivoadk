#include "core/agent_core.h"
#include "logic/random_logic.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    AgentContext ctx;

    agent_init(&ctx, argc, argv, "Randomizer", "Play random legal moves");

    Agent* agent = agent_create_random((uint64_t)(ctx.client.seed + ctx.client.player_number));
    if (!agent) {
        fprintf(stderr, "Failed to create agent\n");
        return EXIT_FAILURE;
    }

    agent_play_loop(&ctx, agent);

    agent->free(agent);

    return EXIT_SUCCESS;
}
