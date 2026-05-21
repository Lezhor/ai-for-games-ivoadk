#include "random_logic.h"
#include "utils/lcg.h"
#include <stdlib.h>

typedef struct {
    lcg_t rng;
} RandomState;

static uint8_t random_get_move(Agent* self, const GameSettings* settings, const GameState* game, uint8_t player_id, uint64_t deadline_ms) {
    (void)settings;
    (void)player_id;
    (void)deadline_ms;
    RandomState* state = (RandomState*)self->internal_state;

    uint8_t free_cells[BOARD_SIZE];
    int free_count = 0;

    for (int i = 0; i < BOARD_SIZE; i++) {
        if ((game->v & (3ULL << (i * 2))) == 0) {
            free_cells[free_count++] = (uint8_t)i;
        }
    }

    if (free_count == 0) {
        return ILLEGAL_MOVE;
    }

    int random_index = lcg_next_int_n(&state->rng, free_count);
    return free_cells[random_index];
}

static void random_free(Agent* self) {
    if (self->internal_state) {
        free(self->internal_state);
    }
    free(self);
}

Agent* agent_create_random(uint64_t seed) {
    Agent* agent = malloc(sizeof(Agent));
    if (!agent) return NULL;

    RandomState* state = malloc(sizeof(RandomState));
    if (!state) {
        free(agent);
        return NULL;
    }

    lcg_set_seed(&state->rng, seed);

    agent->internal_state = state;
    agent->get_move = random_get_move;
    agent->free = random_free;

    return agent;
}
