#ifndef ALPHA_ZERO_MCTS_H
#define ALPHA_ZERO_MCTS_H

#include "game/game.h"
#include "agent/eval/alpha_zero/evaluator_alpha_zero.h"
#include "utils/lcg.h"

#define MCTS_NODE_POOL_SIZE 100000

typedef struct {
    uint32_t visits;
    double value_sum[3];
    double prior_prob;
    int32_t parent_idx;
    int32_t first_child_idx;
    int32_t sibling_idx;
    uint8_t move_id;
} MCTSNode;

typedef struct {
    MCTSNode pool[MCTS_NODE_POOL_SIZE];
    uint32_t pool_cursor;
    int32_t path_buffer[256];
    lcg_t rng;
    double c_puct;
} MCTSContext;

void mcts_init_context(MCTSContext* ctx, uint64_t seed, double c_puct);
void mcts_reset_tree(MCTSContext* ctx);

/**
 * Performs a single MCTS simulation: Selection -> Expansion -> Backprop
 */
void mcts_iteration(MCTSContext* ctx,
                    const GameSettings* settings,
                    const GameState* game,
                    AlphaZeroEvaluator* eval);

/**
 * Final move selection based on visit counts.
 */
uint8_t mcts_select_best_move(MCTSContext* ctx);

#endif // ALPHA_ZERO_MCTS_H
