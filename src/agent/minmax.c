#include "helper/agent_core.h"
#include "helper/minmax_helper.h"
#include <stdio.h>
#include <stdlib.h>

uint8_t minmax_strategy(AgentContext* ctx, void* strategy_data) {
    (void)strategy_data;
    uint8_t max_player = (uint8_t)(ctx->client.player_number + 1);

    uint64_t deadline = ctx->client.input_request_timestamp + (uint64_t)ctx->client.time_limit_sec * 1000 - (uint64_t)ctx->client.latency_ms - 50;

    int depth_reached = 0;
    uint8_t move = minmax_search_iterative(&ctx->game_settings, &ctx->game, max_player, deadline, &depth_reached);

#ifdef NDEBUG
    printf("Depth reached: %d\n", depth_reached);
#endif

    return move;
}

int main(int argc, char *argv[]) {
    AgentContext ctx;

    agent_init(&ctx, argc, argv, "MinMax Paranoid", "Simple MinMax search with Alpha-Beta Pruning. Treats both opponents as one...");

    agent_loop(&ctx, minmax_strategy, NULL);

    // TODO: agent_finish() function which prints what player we were and who won

    return EXIT_SUCCESS;
}
