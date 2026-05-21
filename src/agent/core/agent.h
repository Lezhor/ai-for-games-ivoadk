#ifndef AGENT_H
#define AGENT_H

#include "game/game.h"
#include <stdint.h>

typedef struct Agent Agent;

struct Agent {
    /**
     * Internal state of the agent (e.g., Evaluator, search depth, RNG seed, search tree).
     */
    void* internal_state;

    /**
     * Core decision function: returns a move index for the given board state.
     * @param player_id The player ID of the agent (1, 2, or 3).
     * @param time_deadline Absolute timestamp (ms) by which the move must be returned.
     */
    uint8_t (*get_move)(Agent* self, const GameSettings* settings, const GameState* game, uint8_t player_id, uint64_t time_deadline);

    /**
     * Frees all resources associated with the agent.
     */
    void (*free)(Agent* self);
};

#endif // AGENT_H
