#ifndef ALPHA_ZERO_LOGIC_H
#define ALPHA_ZERO_LOGIC_H

#include "agent/core/agent.h"
#include "agent/eval/alpha_zero/evaluator_alpha_zero.h"

/**
 * Creates an AlphaZero MCTS agent.
 * @param eval The AlphaZero evaluator (NN or hardcoded).
 * @param seed Seed for the MCTS RNG.
 * @param c_puct Exploration constant.
 * @param max_mcts_iterations Maximum number of simulations per move (capped by deadline).
 */
Agent* agent_create_alpha_zero(AlphaZeroEvaluator* eval, uint64_t seed, double c_puct, int max_mcts_iterations);

#endif // ALPHA_ZERO_LOGIC_H
