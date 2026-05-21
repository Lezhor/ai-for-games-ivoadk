#include "agent_core.h"
#include <stdio.h>
#include <stdlib.h>

void agent_init(AgentContext* ctx, int argc, char* argv[], const char* agent_name, const char* strategy_description) {
    ctx->config = parse_args(argc, argv);

    printf("--- IVOADK %s Client ---\n", agent_name);
    printf("Target Host: %s\n", ctx->config.host);
    printf("Target Port: %d\n", ctx->config.port);
    printf("Agent Name: %s\n", ctx->config.agent_name);
    if (strategy_description) {
        printf("Strategy: %s\n", strategy_description);
    }
    printf("----------------------------\n");

    network_client_connect(&ctx->config, &ctx->client);

    ctx->game.v = GAME_STATE_DEFAULT_VALUE;
    game_init_settings(ctx->client.seed, &ctx->game_settings);

    char game_str[128];
    game_to_string(&ctx->game, game_str);
    printf("Init S.:  %s\n", game_str);

#ifdef USE_TUI
    char board_str[256];
    game_to_2d_board_string(&ctx->game_settings, &ctx->game, board_str);
    printf("Initial Board:\n%s\n", board_str);
#endif
}

void agent_play_loop(AgentContext* ctx, Agent* agent) {
    char game_str[128];
#ifdef USE_TUI
    char board_str[256];
#endif

    while (1) {
        while (game_network_receive_move(&ctx->game_settings, &ctx->client, &ctx->game)) {
            game_to_string(&ctx->game, game_str);
            printf("Move %3d: %s, main_player: %d\n", game_get_move_count(&ctx->game), game_str, ctx->client.player_number + 1);
#ifdef USE_TUI
            game_to_2d_board_string(&ctx->game_settings, &ctx->game, board_str);
            printf("%s\n", board_str);
#endif
        }

        uint64_t deadline = ctx->client.input_request_timestamp + (uint64_t)ctx->client.time_limit_sec * 1000 - (uint64_t)ctx->client.latency_ms - 50;
        uint8_t move = agent->get_move(agent, &ctx->game_settings, &ctx->game, (uint8_t)(ctx->client.player_number + 1), deadline);
        game_network_send_move(&ctx->game_settings, &ctx->client, move);
    }
}
