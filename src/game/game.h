#ifndef GAME_H
#define GAME_H

#include "game/board.h"
#include "network/network_client.h"
#include <assert.h>
#include <stdint.h>

typedef union {
    uint64_t v;
    struct {
        uint64_t clear_mask : 38; // Bits 0-37 (which 3 cells to clear during apply)
        uint64_t shift_L    : 6;  // Bits 38-43
        uint64_t shift_M    : 6;  // Bits 44-49
        uint64_t shift_H    : 6;  // Bits 50-55
        uint64_t jump_back  : 5;  // Bits 56-60 // earliest triangle to jump back too (in case rules change)
        uint64_t unused     : 3;  // Bits 61-63
    };
} Triangle;

typedef union {
    uint64_t v;
    struct {
        uint64_t board           : 38; // Bits 0-37:  The 19 cells (2 bits each)
        uint64_t p1_active       : 1;  // Bit 38:     Player 1 active flag
        uint64_t p2_active       : 1;  // Bit 39:     Player 2 active flag
        uint64_t p3_active       : 1;  // Bit 40:     Player 3 active flag
        uint64_t player_turn     : 2;  // Bits 41-42: who's turn is it
        uint64_t p1_score        : 7;  // Bits 43-49: Player 1 score
        uint64_t p2_score        : 7;  // Bits 50-56: Player 2 score
        uint64_t p3_score        : 7;  // Bits 57-63: Player 3 score
    };
} GameState;

typedef struct {
    // uint8_t main_player;

    board_height_t board_heights[BOARD_SIZE];
    uint8_t board_inverse_map[BOARD_SIZE]; // value of index i should be index in board_heights where value is i

    Triangle triangles[BOARD_TRIANGLE_COUNT];

} GameSettings;

#define GAME_MASK_BOARD            (uint64_t) 0x0000003FFFFFFFFF
#define GAME_MASK_BOARD_EVEN       (uint64_t) 0x0000001555555555 // bits 0, 2, 4, 6, ...
#define GAME_MASK_BOARD_ODD        (uint64_t) 0x0000002AAAAAAAAA // bits 1, 3, 5, 7, ...
#define GAME_MASK_ACTIVE_PLAYERS   (uint64_t) 0x000001C000000000
#define GAME_MASK_PLAYER_TURN      (uint64_t) 0x0000060000000000
#define GAME_MASK_SCORES           (uint64_t) 0xFFFFF8C000000000

#define GAME_SCORE_TO_WIN          12

// board empty, all players active, 1st players turn, 0 score
#define GAME_STATE_DEFAULT_VALUE   (uint64_t) 0x000003C000000000

void game_init_settings(int32_t seed, GameSettings* out_game_settings);

void game_take_move(const GameSettings* game_settings, GameState* game, uint8_t player, uint8_t move);
int game_is_move_valid(const GameState* game, uint8_t move);

int game_finished_condition(GameState* game);
void get_tournament_scores(GameState* game, uint8_t out_scores[4]);
uint8_t game_get_winner(GameState* game, uint8_t* out_winner_score);
int game_get_move_count(GameState* game);
void game_cycle_perspective(GameState* game, uint8_t from, uint8_t to);

// network wrapper functions:

uint8_t game_network_player_from_net(const NetworkClient* client, uint8_t player);
uint8_t game_network_player_to_net(const NetworkClient* client, uint8_t player);
uint8_t game_network_move_index_from_net(const GameSettings* game_settings, uint8_t move);
uint8_t game_network_move_index_to_net(const GameSettings* game_settings, uint8_t move);
int game_network_receive_move(const GameSettings* game_settings, NetworkClient* client, GameState* game);
void game_network_send_move(const GameSettings* game_settings, NetworkClient* client, uint8_t move);

// ui & strings

void game_to_string(const GameState* game, char* out_str);
void game_to_2d_board_string(const GameSettings* game_settings, const GameState* game, char* out_str);

#endif
