#ifndef TRAIN_CORE_H
#define TRAIN_CORE_H

#include <stdbool.h>
#ifdef AGENT_TRAINING
#include "agent/core/agent.h"
#include "agent/eval/evaluator.h"
#include "utils/lcg.h"

/**
 * Generic parameters for evolutionary training.
 */
typedef struct {
    double mutation_rate;
    double mutation_scale;
    const void* template_state;
    lcg_t* rng;
    bool use_gaussian;
} EAMutationParams;

/**
 * Function pointer type for creating an agent with a specific evaluator.
 */
typedef Agent* (*AgentFactory)(Evaluator** out_eval);

/**
 * Runs the Evolutionary Algorithm training loop.
 * @param argc CLI argument count
 * @param argv CLI argument array
 * @param factory Function that creates an agent and returns its evaluator for mutation
 * @param model_path The path to save the best model to.
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int run_ea_training_loop(int argc, char* argv[], AgentFactory factory, const char* model_path);
#endif

#endif // TRAIN_CORE_H
