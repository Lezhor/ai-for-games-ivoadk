#ifndef EVAL_MINMAX_HARDCODED_H
#define EVAL_MINMAX_HARDCODED_H

#include "agent/eval/evaluator.h"

/**
 * Creates an evaluator for MinMax that uses the current hardcoded heuristic logic.
 * Logic: 2 * my_score - opponent_score_sum
 */
Evaluator* evaluator_create_minmax_hardcoded(void);

#endif // EVAL_MINMAX_HARDCODED_H
