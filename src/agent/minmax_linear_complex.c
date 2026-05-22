#include "core/agent_core.h"
#include "logic/minmax_logic.h"
#include "eval/minmax/eval_minmax_linear_complex.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef AGENT_TRAINING
#include "train/train_core.h"

static Agent* minmax_linear_complex_factory(Evaluator** out_eval) {
    *out_eval = evaluator_create_minmax_linear_complex(NULL);
    return agent_create_minmax(*out_eval, 4, false); // Lower depth for faster training
}
#endif

int main(int argc, char *argv[]) {
#ifdef AGENT_TRAINING
    return run_ea_training_loop(argc, argv, minmax_linear_complex_factory, "models/minmax_linear_complex/best.txt");
#else
    // 1. Create the evaluator
    // Try to load the trained model if it exists, otherwise fallback to defaults
    Evaluator* eval = evaluator_create_minmax_linear_complex("models/minmax_linear_complex/best.txt");
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
    agent_init(&ctx, argc, argv, "MinMax Linear Complex", "MinMax search with Alpha-Beta Pruning using a complex trained linear heuristic.");

    // 4. Run the network play loop
    agent_play_loop(&ctx, agent);

    // 5. Cleanup
    agent->free(agent);
    eval->free(eval);

    return EXIT_SUCCESS;
#endif
}
