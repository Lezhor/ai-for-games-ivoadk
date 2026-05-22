#include "core/agent_core.h"
#include "logic/minmax_logic.h"
#include "eval/minmax/eval_minmax_hardcoded.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // 1. Create the evaluator
    Evaluator* eval = evaluator_create_minmax_hardcoded();
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
    agent_init(&ctx, argc, argv, "MinMax Hardcoded", "MinMax search with Alpha-Beta Pruning using a hardcoded heuristic.");

    // 4. Run the network play loop
    agent_play_loop(&ctx, agent);

    // 5. Cleanup
    agent->free(agent);
    eval->free(eval);

    return EXIT_SUCCESS;
}
