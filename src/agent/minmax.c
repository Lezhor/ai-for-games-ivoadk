#include "helper/agent_core.h"
#include "helper/minmax_helper.h"
#include <stdio.h>
#include <stdlib.h>

uint8_t minmax_strategy(AgentContext* ctx, void* strategy_data) {
    int depth = *(int*)strategy_data;
    uint8_t max_player = (uint8_t)(ctx->client.player_number + 1);

    return minmax_search(&ctx->game_settings, &ctx->game, depth, max_player);
}

int main(int argc, char *argv[]) {
    AgentContext ctx;

    agent_init(&ctx, argc, argv, "MinMax Paranoid", "Simple MinMax search with Alpha-Beta Pruning. Treats both opponents as one...");

    int depth = 3;
    if (argc > 4) {
        depth = atoi(argv[4]);
    }
    printf("Search Depth: %d\n", depth);

    agent_loop(&ctx, minmax_strategy, &depth);

    return EXIT_SUCCESS;
}
