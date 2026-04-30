#include "helper/agent_core.h"
#include "utils/lcg.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    lcg_t rng;
} RandomizerData;

uint8_t get_random_free(const GameState* game, lcg_t* rng) {
    uint8_t free_cells[BOARD_SIZE];
    int free_count = 0;

    for (int i = 0; i < BOARD_SIZE; i++) {
        if ((game->v & (3ULL << (i * 2))) == 0) {
            free_cells[free_count++] = (uint8_t)i;
        }
    }

    if (free_count == 0) {
        return BOARD_SIZE;
    }

    int random_index = lcg_next_int_n(rng, free_count);
    return free_cells[random_index];
}

uint8_t randomizer_strategy(AgentContext* ctx, void* strategy_data) {
    RandomizerData* data = (RandomizerData*)strategy_data;
    return get_random_free(&ctx->game, &data->rng);
}

int main(int argc, char *argv[]) {
    AgentContext ctx;
    RandomizerData data;

    agent_init(&ctx, argc, argv, "Randomizer", "Play random legal moves");

    // added +player cuz else all randomizers have the same rng object :/
    lcg_set_seed(&data.rng, (uint64_t)(ctx.client.seed + ctx.client.player_number));

    agent_loop(&ctx, randomizer_strategy, &data);

    return EXIT_SUCCESS;
}
