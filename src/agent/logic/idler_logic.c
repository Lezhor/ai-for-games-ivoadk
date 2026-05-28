#include "idler_logic.h"
#include "game/game_internal.h"
#include <stdlib.h>

static uint8_t idler_get_move(Agent* self, const GameSettings* settings, const GameState* game, uint8_t player_id, uint64_t time_deadline) {
    (void)self; (void)settings; (void)game; (void)player_id; (void)time_deadline;
    return ILLEGAL_MOVE;
}

static void idler_free(Agent* self) {
    free(self);
}

Agent* agent_create_idler(void) {
    Agent* agent = malloc(sizeof(Agent));
    if (!agent) return NULL;
    agent->internal_state = NULL;
    agent->get_move = idler_get_move;
    agent->free = idler_free;
    return agent;
}
