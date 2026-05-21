#ifndef MINMAX_LOGIC_H
#define MINMAX_LOGIC_H

#include "agent/core/agent.h"
#include "agent/eval/evaluator.h"
#include <stdbool.h>

/**
 * Creates a MinMax agent.
 * @param eval The evaluator to use at leaf nodes.
 * @param max_depth Maximum search depth.
 * @param use_iterative_deepening Whether to use iterative deepening.
 */
Agent* agent_create_minmax(Evaluator* eval, int max_depth, bool use_iterative_deepening);

#endif // MINMAX_LOGIC_H
