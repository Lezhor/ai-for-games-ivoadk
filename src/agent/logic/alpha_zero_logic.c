#include "alpha_zero_logic.h"
#include "alpha_zero_mcts.h"
#include "utils/time_utils.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    AlphaZeroEvaluator* eval;
    MCTSContext mcts_ctx;
    int max_iterations;
} AlphaZeroAgentState;

static uint8_t az_get_move(Agent* self, const GameSettings* settings, const GameState* game, uint8_t player_id, uint64_t time_deadline) {
    (void)player_id;
    AlphaZeroAgentState* state = (AlphaZeroAgentState*)self->internal_state;

    mcts_reset_tree(&state->mcts_ctx);

    int iterations = 0;
    // Buffer time: finish 10ms before deadline to be safe
    uint64_t safe_deadline = (time_deadline > 10) ? time_deadline - 10 : time_deadline;

    while (iterations < state->max_iterations) {
        // Check deadline every 32 iterations to save time_get_now_ms calls
        if ((iterations & 31) == 0) {
            if (time_get_now_ms() >= safe_deadline) break;
        }

        mcts_iteration(&state->mcts_ctx, settings, game, state->eval);
        iterations++;
    }

#ifndef NDEBUG
    printf("MCTS Iterations: %d\n", iterations);
#endif
    return mcts_select_best_move(&state->mcts_ctx);
}

static void az_free(Agent* self) {
    if (!self) return;
    AlphaZeroAgentState* state = (AlphaZeroAgentState*)self->internal_state;
    // Note: We don't free eval here because it might be shared or managed elsewhere
    // but the Agent structure usually owns its evaluator in other implementations.
    // To match minmax, we'll assume the creator handles eval lifecycle if needed,
    // or we free it here if this agent "owns" it.
    if (state) {
        if (state->eval) state->eval->free(state->eval);
        free(state);
    }
    free(self);
}

Agent* agent_create_alpha_zero(AlphaZeroEvaluator* eval, uint64_t seed, double c_puct, int max_mcts_iterations) {
    Agent* agent = malloc(sizeof(Agent));
    if (!agent) return NULL;

    AlphaZeroAgentState* state = malloc(sizeof(AlphaZeroAgentState));
    if (!state) {
        free(agent);
        return NULL;
    }

    state->eval = eval;
    state->max_iterations = max_mcts_iterations;
    mcts_init_context(&state->mcts_ctx, seed, c_puct);

    agent->internal_state = state;
    agent->get_move = az_get_move;
    agent->free = az_free;

    return agent;
}
