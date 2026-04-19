#ifndef GAME_INTERNAL_H
#define GAME_INTERNAL_H

#include "game/game.h"

static inline int game_is_player_active(const GameState* game, uint8_t player) {
    assert(player >= 1 && player <= 3 && "player out of bounds in game_is_player_active()");
    return (game->v >> (37 + player)) & 1;
}

static inline void game_set_player_active(GameState* game, uint8_t player) {
    assert(player >= 1 && player <= 3 && "player out of bounds in game_set_player_inactive()");
    game->v |= (uint64_t)1 << (37 + player);
}

static inline void game_set_player_inactive(GameState* game, uint8_t player) {
    assert(player >= 1 && player <= 3 && "player out of bounds in game_set_player_inactive()");
    game->v &= ~((uint64_t)1 << (37 + player));
}


void game_init_triangles(board_height_t* board_heights, Triangle* out_triangles);

void game_apply_move(GameState* game, uint8_t player, uint8_t move);
void game_apply_triangles(const GameSettings* game_settings, GameState* game);

void game_turn_set(GameState* game, uint8_t player);
void game_turn_advance(GameState* game);

#endif
