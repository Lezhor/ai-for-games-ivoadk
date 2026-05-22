#include "core/agent_core.h"
#include "logic/top_logic.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    Agent* agent = agent_create_top_picker();
    if (!agent) {
        fprintf(stderr, "Failed to create agent\n");
        return EXIT_FAILURE;
    }

    AgentContext ctx = {0};
    ctx.config.icon_path = "./assets/icons/tree.b64";
    agent_init(&ctx, argc, argv, "Top Picker", "Always picks the highest available cell index.");

    agent_play_loop(&ctx, agent);

    agent->free(agent);

    return EXIT_SUCCESS;
}
