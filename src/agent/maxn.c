#include "helper/agent_core.h"
#include "helper/maxn_helper.h"
#include <stdio.h>
#include <stdlib.h>

uint8_t maxn_strategy(AgentContext* ctx, void* strategy_data) {
    int depth = *(int*)strategy_data;
    uint8_t max_player = (uint8_t)(ctx->client.player_number + 1);

    return maxn_search(&ctx->game_settings, &ctx->game, depth, max_player);
}

int main(int argc, char *argv[]) {
    AgentContext ctx;

    agent_init(&ctx, argc, argv, "MaxN", "MaxN search with Immediate Win Pruning. Each player maximizes their own score...");

    int depth = 6;
    if (argc > 4) {
        depth = atoi(argv[4]);
    }
    printf("Search Depth: %d\n", depth);

    agent_loop(&ctx, maxn_strategy, &depth);

    return EXIT_SUCCESS;
}
