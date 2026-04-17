#include "game/game.h"
#include "game/board.h"
#include "utils/array_utils.h"
#include "utils/lcg.h"
#include <assert.h>

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

/**
 * applies single move without calculating scores etc.
 */
void game_apply_move(GameState* game, uint8_t player, uint8_t move) {
    assert(move < BOARD_SIZE && "illegal move in game_apply_move()");
    assert(player >= 1 && player <= 3 && "illegal player value in game_apply_move()");
    assert(game_is_player_active(game, player) && "inactive player tried to apply move in game_apply_move()");
    assert(((game->v & ((uint64_t)3 << (move * 2))) == 0) && "cell already taken in game_apply_move()");
    game->v |= (uint64_t)player << (move * 2);
}

void game_apply_triangles(const GameSettings* game_settings, GameState* game) {
    (void)game_settings;
    (void)game;
    // TODO: implement apply triangles
}

/**
 * applies move to gamestate.
 *
 * Note:
 * game holds who's turn it is.
 * If the next player is NOT the one whose turn its rn it marks the skipped player(s)
 * as inactive because they probably got kicked.
 */
void game_take_move(const GameSettings* game_settings, GameState* game, uint8_t player, uint8_t move) {
    // move == BOARD_SIZE is fine cuz its considered an intentionally illegal move
    assert(move <= BOARD_SIZE && "move out of bounds in game_take_move");
    assert(game_is_player_active(game, player) && "inactive player tried to take move in game_take_move()");

    game_turn_set(game, player); // inactivates players who got skipped
    if (move == BOARD_SIZE) {
        // intentionally played illegal move
        game_set_player_inactive(game, player);
    } else {
        game_apply_move(game, player, move);
        game_apply_triangles(game_settings, game);
    }
    game_turn_advance(game);
}

/**
 * sets who's turn its rn.
 * this is NOT for incrementing a turn! whenever current turn isnt passed player it inactivates all players in between
 * if current turn is player turn then does nothing (cuz its already set)
 * if players skipped then sets to inactive.
 * if 1 player behind - inactivates this player
 * if 2 players behind - inactivates both other players
 */
void game_turn_set(GameState* game, uint8_t player) {
    assert(player >= 1 && player <= 3 && "player out of bounds in game_set_turn()");
    assert(game_is_player_active(game, player) && "tried to set player turn to inactive player in game_set_turn()");

    //  target:          3  2  1  0
    // current turn 3:   0  5  4  0
    // current turn 2:   2  0  6  0
    // current turn 1:   3  1  0  0
    // current turn 0:   0  0  0  0
    // e.g. from 1 to 1 its 0 aka 000 - so we don't change anything (we are already at correct turn)
    // e.g. from 3 to 2 its 6 aka 110 - so we inactivate bit 2, 3 since they didnt take a move
    // e.g. from 2 to 3 its 2 aka 010 - so we inactivate bit 2 since 2 didnt take a move
    uint64_t inactivate_lut = 0x0540206031000000ULL;

    int index = (game->player_turn << 2) | player;
    uint64_t skip_mask = (inactivate_lut >> (index * 4)) & 0xF;
    game->v &= ~(skip_mask << 38);

    game->player_turn = player & 3;
}

/**
 * advances to next actives player turn
 * cycles between 1,2,3 if all players active.
 */
void game_turn_advance(GameState* game) {
    // holy shit - we have 3 input bits for active players, 2 input bits for current player turn
    // this means we have 5 input bits or 2^5=32 scenarios we need outputs for.
    // we have 2 output bits per scenario - so it fits EXACTLY in 64 bits once again!
    // what are the odds!!!???

    assert((game->v & GAME_MASK_ACTIVE_PLAYERS) != 0 && "no active players in game_turn_advance()");

    // NOTE: i might be able to cut off even more CPU cycles if i do (player_turn | active) the other way around cuz they are in this order in the game state. (only 1 fetch instead of two)

    // active         mappings          |  output bits  | HEX
    //   111:   3->1, 2->3, 1->2, 0->1  |  01 11 10 01  |  79
    //   110:   3->2, 2->3, 1->2, 0->2  |  10 11 10 10  |  BA
    //   101:   3->1, 2->3, 1->3, 0->1  |  01 11 11 01  |  7D
    //   100:   3->3, 2->3, 1->3, 0->3  |  11 11 11 11  |  FF
    //   011:   3->1, 2->1, 1->2, 0->1  |  01 01 10 01  |  59
    //   010:   3->2, 2->2, 1->2, 0->2  |  10 10 10 10  |  AA
    //   001:   3->1, 2->1, 1->1, 0->1  |  01 01 01 01  |  55
    //   000:   3->3, 2->2, 1->1, 0->0  |  11 10 01 00  |  E4
    uint64_t next_turn_lut = 0x79BA7DFF59AA55E4ULL;

    // 36 instead of 38 - already shifted << 2 to not shift twice :)
    uint64_t active_shifted = (game->v >> 36) & 0x1C;
    uint64_t index = active_shifted | game->player_turn;
    game->player_turn = (next_turn_lut >> (index * 2)) & 3;
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
