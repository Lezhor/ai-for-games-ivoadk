#include "core/agent_core.h"
#include "logic/alpha_zero_logic.h"
#include "agent/eval/alpha_zero/evaluator_alpha_zero.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef AGENT_TRAINING
#include "train/alpha_zero/train_alpha_zero.h"
#endif

int main(int argc, char* argv[]) {

#ifdef AGENT_TRAINING
    TrainAlphaZeroConfig az_config = {0};
    parse_alpha_zero_args(argc, argv, &az_config);
    if (az_config.training_data_output == NULL) {
        fprintf(stderr, "Error: --training-data-output <path> is required for training mode.\n");
        return EXIT_FAILURE;
    }
    run_alpha_zero_training_loop(&az_config);
    return EXIT_SUCCESS;
#else
    // 1. Parse AlphaZero specific args first (to check if we even can start)
    TrainAlphaZeroConfig az_config = {0};
    parse_alpha_zero_args(argc, argv, &az_config);

    // 2. Create the evaluator
    AlphaZeroEvaluator* eval = az_config.use_nn ?
        evaluator_create_alpha_zero_nn(az_config.model_path) :
        evaluator_create_alpha_zero_hardcoded();

    if (!eval) {
        fprintf(stderr, "Failed to create AlphaZero evaluator. Check model-path.\n");
        return EXIT_FAILURE;
    }

    // 3. Create the agent
    uint64_t seed = az_config.seed_provided ? az_config.seed : (uint64_t)time(NULL);
    Agent* agent = agent_create_alpha_zero(eval, seed, az_config.c_puct, az_config.num_mcts_iterations);
    
    if (!agent) {
        fprintf(stderr, "Failed to create agent\n");
        eval->free(eval);
        return EXIT_FAILURE;
    }

    // 4. Initialize network context and connect (Last thing before the loop)
    AgentContext ctx = {0};
    ctx.config.icon_path = "./assets/icons/kanji_ai.b64";
    agent_init(&ctx, argc, argv, "AlphaZero", "MCTS search powered by Neural Network evaluation.");

    // 5. Run the network play loop
    agent_play_loop(&ctx, agent);

    // 6. Cleanup
    agent->free(agent);
    
    return EXIT_SUCCESS;
#endif
}
