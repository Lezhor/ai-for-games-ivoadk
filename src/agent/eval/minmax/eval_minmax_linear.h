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
/**
 * Parameters for evolutionary training of the linear evaluator.
 */
typedef struct {
    double mutation_rate;
    double mutation_scale;
    const LinearEvalState* template_state; // If non-NULL, copy this state before mutating
} EAMutationParams;
#endif

/**
 * Creates a linear evaluator for MinMax.
 * @param load_path Optional path to load weights from. If NULL, weights are initialized to default values.
 * @return A new Evaluator instance.
 */
Evaluator* evaluator_create_minmax_linear(const char* load_path);

#endif // EVAL_MINMAX_LINEAR_H
