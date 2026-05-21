#include "core/agent_core.h"
#include "logic/maxn_logic.h"
#include "eval/maxn/eval_maxn_hardcoded.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // 1. Create the evaluator
    Evaluator* eval = evaluator_create_maxn_hardcoded();
    if (!eval) {
        fprintf(stderr, "Failed to create evaluator\n");
        return EXIT_FAILURE;
    }

    int depth = 6;
    if (argc > 4) {
        depth = atoi(argv[4]);
    }
    printf("Search Depth: %d\n", depth);

    // 2. Create the agent
    Agent* agent = agent_create_maxn(eval, depth);
    if (!agent) {
        fprintf(stderr, "Failed to create agent\n");
        eval->free(eval);
        return EXIT_FAILURE;
    }

    // 3. Initialize network context
    AgentContext ctx;
    agent_init(&ctx, argc, argv, "MaxN", "MaxN search with Immediate Win Pruning. Each player maximizes their own score...");

    // 4. Run the network play loop
    agent_play_loop(&ctx, agent);

    // 5. Cleanup
    agent->free(agent);
    eval->free(eval);

    return EXIT_SUCCESS;
}
