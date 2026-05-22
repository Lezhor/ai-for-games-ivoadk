#ifndef TRAIN_CORE_H
#define TRAIN_CORE_H

#ifdef AGENT_TRAINING
#include "agent/core/agent.h"
#include "agent/eval/evaluator.h"


/**
 * Function pointer type for creating an agent with a specific evaluator.
 */
typedef Agent* (*AgentFactory)(Evaluator** out_eval);

/**
 * Runs the Evolutionary Algorithm training loop.
 * @param argc CLI argument count
 * @param argv CLI argument array
 * @param factory Function that creates an agent and returns its evaluator for mutation
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int run_ea_training_loop(int argc, char* argv[], AgentFactory factory);
#endif

#endif // TRAIN_CORE_H
