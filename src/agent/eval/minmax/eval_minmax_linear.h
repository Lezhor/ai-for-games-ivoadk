#ifndef EVAL_MINMAX_LINEAR_H
#define EVAL_MINMAX_LINEAR_H

#include "agent/eval/evaluator.h"

/**
 * Linear evaluation state.
 * weights[0]: weight for the perspective player's score.
 * weights[1]: weight for the next player's score.
 * weights[2]: weight for the previous player's score.
 */
typedef struct {
    double weights[3];
} LinearEvalState;

#ifdef AGENT_TRAINING
#include "train/train_core.h"
#endif

/**
 * Creates a linear evaluator for MinMax.
 * @param load_path Optional path to load weights from. If NULL, weights are initialized to default values.
 * @return A new Evaluator instance.
 */
Evaluator* evaluator_create_minmax_linear(const char* load_path);

#endif // EVAL_MINMAX_LINEAR_H
