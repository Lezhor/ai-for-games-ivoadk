#include "network/network_client.h"
#include "cli.h"
#include "game/game.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef AGENT_TRAINING
#include "train/alpha_zero/train_alpha_zero.h"
#endif

int main(int argc, char* argv[]) {
    AgentConfig config = {0};
    
    // Parse AlphaZero specific flags first
    parse_alpha_zero_args(argc, argv, &config);
    
    // Parse network flags (ignoring unknown)
    parse_args(argc, argv, &config);

#ifdef AGENT_TRAINING
    if (config.training_data_output == NULL) {
        fprintf(stderr, "Error: --training-data-output <path> is required for training mode.\n");
        return 1;
    }
    run_alpha_zero_training_loop(&config);
#else
    printf("Starting AlphaZero Agent: %s\n", config.agent_name);
    // TODO: Implement AlphaZero network play logic
    // This will involve network_client_connect and the standard game loop 
    // using mcts_get_move (once refactored to be shared).
    printf("Network play not yet implemented for AlphaZero.\n");
#endif

    return 0;
}
