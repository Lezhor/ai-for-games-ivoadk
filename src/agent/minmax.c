#include "helper/agent_core.h"
#include <stdio.h>
#include <stdlib.h>

uint8_t minmax_strategy(AgentContext* ctx, void* strategy_data) {
    (void)ctx;
    (void)strategy_data;

    // TODO: implement minmax

    return BOARD_SIZE;
}

int main(int argc, char *argv[]) {
    AgentContext ctx;

    agent_init(&ctx, argc, argv, "MinMax Paranoid", "Simple MinMax search with Alpha-Beta Pruning. Treats both opponents as one...");

    // TODO: strategy_data should contain smth. like search depth maybe? fetch it from cli aswell :)
    agent_loop(&ctx, minmax_strategy, NULL);

    return EXIT_SUCCESS;
}
