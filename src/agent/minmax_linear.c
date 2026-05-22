#include "core/agent_core.h"
#include "logic/minmax_logic.h"
#include "eval/minmax/eval_minmax_linear.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef AGENT_TRAINING
#include "train/train_core.h"

static Agent* minmax_linear_factory(Evaluator** out_eval) {
    *out_eval = evaluator_create_minmax_linear(NULL);
    return agent_create_minmax(*out_eval, 4, false); // Lower depth for faster training
}
#endif

int main(int argc, char *argv[]) {
#ifdef AGENT_TRAINING
    return run_ea_training_loop(argc, argv, minmax_linear_factory);
#else
    // 1. Create the evaluator
    // Try to load the trained model if it exists, otherwise fallback to defaults
    Evaluator* eval = evaluator_create_minmax_linear("models/minmax/best_linear.txt");
    if (!eval) {
        fprintf(stderr, "Failed to create evaluator\n");
        return EXIT_FAILURE;
    }

    // 2. Create the agent
    Agent* agent = agent_create_minmax(eval, 64, true); // 64 depth with iterative deepening
    if (!agent) {
        fprintf(stderr, "Failed to create agent\n");
        eval->free(eval);
        return EXIT_FAILURE;
    }

    // 3. Initialize network context
    AgentContext ctx = {0};
    ctx.config.icon_path = "./assets/icons/smiley.b64";
    agent_init(&ctx, argc, argv, "MinMax Linear", "MinMax search with Alpha-Beta Pruning using a trained linear heuristic.");

    // 4. Run the network play loop
    agent_play_loop(&ctx, agent);

    // 5. Cleanup
    agent->free(agent);
    eval->free(eval);

    return EXIT_SUCCESS;
#endif
}
