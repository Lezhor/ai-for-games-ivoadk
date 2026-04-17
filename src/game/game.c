
#include "game/game.h"
#include "game/board.h"
#include "utils/array_utils.h"
#include "utils/lcg.h"

void game_init_settings(int32_t seed, GameSettings* out_game_settings) {

    // init board heights
    for (int i = 0; i < BOARD_SIZE; i++) {
        out_game_settings->board_heights[i] = (board_height_t)i;
    }
    lcg_t rng;
    lcg_set_seed(&rng, (uint64_t)seed);
    board_height_array_shuffle(out_game_settings->board_heights, BOARD_SIZE, &rng);
    board_height_calculate_inverse_map(out_game_settings->board_heights, out_game_settings->board_inverse_map, BOARD_SIZE);

    // TODO: init triangles in correct order
}

void game_apply_triangles(GameSettings* game_settings, GameState* game) {
    (void)game_settings;
    (void)game;
}

/**
 * applies move to gamestate.
 *
 * Note:
 * game holds who's turn it is.
 * If the next player is NOT the one whose turn its rn it marks the skipped player(s)
 * as inactive because they probably got kicked.
 */
void game_apply_move(GameSettings* game_settings, GameState* game, uint8_t player, uint8_t move) {
    (void)game_settings;
    (void)game;
    (void)player;
    (void)move;
    // TODO: assert that player is active
    if (move == BOARD_SIZE) {
        // intentionally played illegal move
        // TODO: set player to inactive
        return;
    }
    // TODO: apply move
    game_apply_triangles(game_settings, game);
}

int game_finished_condition(GameState* game) {
    // check if 1 or less players remaining
    if (__builtin_popcountll(game->v & GAME_MASK_ACTIVE_PLAYERS) <= 1) {
        return 1;
    }
    // check if board is full (no need to access game->board here and waste one cpu cycle :)
    if (((game->v | (game->v >> 1)) & GAME_MASK_BOARD_EVEN) == GAME_MASK_BOARD_EVEN) {
        return 1;
    }
    // TODO: maximum score reached condition
    return 0;
}

uint8_t game_get_winner(GameState* game, uint8_t* out_winner_score) {
    // TODO: implement get winner by comparing scores
    (void)(game);
    (void)(out_winner_score);
    return 0;
}

int game_get_move_count(GameState* game) {
    // total obtained score x2 plus the number of stones left on the board.
    return (game->p1_score + game->p2_score + game->p3_score) * 2
        + __builtin_popcountll(((game->v | (game->v >> 1)) & GAME_MASK_BOARD_EVEN));
}
