#include "top_logic.h"
#include <stdlib.h>

static uint8_t top_picker_get_move(Agent* self, const GameSettings* settings, const GameState* game, uint8_t player_id, uint64_t deadline_ms) {
    (void)self;
    (void)settings;
    (void)player_id;
    (void)deadline_ms;

    // Iterate backwards from the highest cell index to find the first free one.
    for (int i = BOARD_SIZE - 1; i >= 0; i--) {
        if ((game->v & (3ULL << (i * 2))) == 0) {
            return (uint8_t)i;
        }
    }

    return ILLEGAL_MOVE;
}

static void top_picker_free(Agent* self) {
    // No internal state to free.
    free(self);
}

Agent* agent_create_top_picker(void) {
    Agent* agent = malloc(sizeof(Agent));
    if (!agent) return NULL;

    agent->internal_state = NULL;
    agent->get_move = top_picker_get_move;
    agent->free = top_picker_free;

    return agent;
}
