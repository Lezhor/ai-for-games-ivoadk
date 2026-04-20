#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "cli.h"
#include "game/board.h"
#include "game/game.h"
#include "network/network_client.h"
#include "utils/lcg.h"

// TODO: this should be passed as a function pointer to a more generic agent loop
int get_random_free(const GameState* game, lcg_t* rng) {
    // not optimized but i don't care rn

    uint8_t free_cells[BOARD_SIZE];
    int free_count = 0;

    for (int i = 0; i < BOARD_SIZE; i++) {
        if ((game->v & (3ULL << (i * 2))) == 0) {
            free_cells[free_count++] = (uint8_t)i;
        }
    }

    if (free_count == 0) {
        return BOARD_SIZE;
    }

    int random_index = lcg_next_int_n(rng, free_count);

    return free_cells[random_index];
}

int main(int argc, char *argv[]) {
    AgentConfig config = parse_args(argc, argv);

    printf("--- IVOADK Randomizer Client ---\n");
    printf("Target Host: %s\n", config.host);
    printf("Target Port: %d\n", config.port);
    printf("Agent Name: %s\n", config.agent_name);
    printf("Strategy: Play random legal moves\n");
    printf("----------------------------\n");

    NetworkClient client;

    network_client_connect(&config, &client);

    GameSettings game_settings;
    GameState game = { .v = GAME_STATE_DEFAULT_VALUE };

    game_init_settings(client.seed, &game_settings);

    char game_str[128];
    game_to_string(&game, game_str);
    printf("Init S.:  %s\n", game_str);

#ifdef USE_TUI
    char board_str[256];
    game_to_2d_board_string(&game_settings, &game, board_str);
    printf("Initial Board:\n%s\n", board_str);
#endif /* ifdef USE_TUI */

    lcg_t rng;
    // added +player cuz else all randomizers have the same rng object :/
    lcg_set_seed(&rng, (uint64_t)(client.seed + client.player_number));

    // game loop

    uint8_t move;
    while (1) {
        while (game_network_receive_move(&game_settings, &client, &game)) {
            game_to_string(&game, game_str);
            printf("Move %3d: %s\n", game_get_move_count(&game), game_str);
#ifdef USE_TUI
            game_to_2d_board_string(&game_settings, &game, board_str);
            printf("%s\n", board_str);
#endif /* ifdef USE_TUI */
        }
        // random move
        move = (uint8_t)get_random_free(&game, &rng);
        game_network_send_move(&game_settings, &client, move);
    }

    return EXIT_SUCCESS;
}
