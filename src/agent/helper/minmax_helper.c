#include "minmax_helper.h"
#include <limits.h>

#define WIN_SCORE 100000
#define LOSS_SCORE -100000

static int evaluate(const GameState* state, uint8_t max_player, int depth) {
    int scores[4] = {0, (int)state->p1_score, (int)state->p2_score, (int)state->p3_score};
    int my_score = scores[max_player];
    
    int max_opp_score = 0;
    for (int i = 1; i <= 3; i++) {
        if (i != (int)max_player && scores[i] > max_opp_score) {
            max_opp_score = scores[i];
        }
    }

    // Check if game finished
    if (game_finished_condition((GameState*)state)) {
        uint8_t win_score_val;
        uint8_t winner = game_get_winner((GameState*)state, &win_score_val);
        if (winner == max_player) return WIN_SCORE + depth + my_score;
        if (winner != 0) return LOSS_SCORE - depth - win_score_val;
    }

    return my_score - max_opp_score;
}

static int minmax_recursive(const GameSettings* settings, GameState* state, int depth, int alpha, int beta, uint8_t max_player) {
    if (depth <= 0 || game_finished_condition(state)) {
        return evaluate(state, max_player, depth);
    }

    uint8_t current_player = (uint8_t)(state->player_turn);
    int is_max_node = (current_player == max_player);

    uint64_t occupied = ((state->v | (state->v >> 1)) & GAME_MASK_BOARD_EVEN);
    uint64_t free_cells = GAME_MASK_BOARD_EVEN & ~occupied;

    if (is_max_node) {
        int max_eval = INT_MIN;
        uint64_t bits = free_cells;
        while (bits) {
            int bit_idx = 63 - __builtin_clzll(bits);
            uint8_t move = (uint8_t)(bit_idx / 2);
            bits &= ~(1ULL << bit_idx);

            GameState next_state = *state;
            game_take_move(settings, &next_state, current_player, move);
            int eval = minmax_recursive(settings, &next_state, depth - 1, alpha, beta, max_player);
            if (eval > max_eval) max_eval = eval;
            if (eval > alpha) alpha = eval;
            if (beta <= alpha) break;
        }

        // Evaluate illegal move last if pruning didn't happen
        if (beta > alpha) {
            GameState next_state = *state;
            game_take_move(settings, &next_state, current_player, ILLEGAL_MOVE);
            int eval = minmax_recursive(settings, &next_state, depth - 1, alpha, beta, max_player);
            if (eval > max_eval) max_eval = eval;
        }

        return max_eval;
    } else {
        int min_eval = INT_MAX;
        uint64_t bits = free_cells;
        while (bits) {
            int bit_idx = 63 - __builtin_clzll(bits);
            uint8_t move = (uint8_t)(bit_idx / 2);
            bits &= ~(1ULL << bit_idx);

            GameState next_state = *state;
            game_take_move(settings, &next_state, current_player, move);
            int eval = minmax_recursive(settings, &next_state, depth - 1, alpha, beta, max_player);
            if (eval < min_eval) min_eval = eval;
            if (eval < beta) beta = eval;
            if (beta <= alpha) break;
        }

        if (beta > alpha) {
            GameState next_state = *state;
            game_take_move(settings, &next_state, current_player, ILLEGAL_MOVE);
            int eval = minmax_recursive(settings, &next_state, depth - 1, alpha, beta, max_player);
            if (eval < min_eval) min_eval = eval;
        }

        return min_eval;
    }
}

uint8_t minmax_search(const GameSettings* settings, const GameState* state, int depth, uint8_t max_player) {
    uint8_t best_move = ILLEGAL_MOVE;
    int max_eval = INT_MIN;
    int alpha = INT_MIN;
    int beta = INT_MAX;

    uint8_t current_player = (uint8_t)(state->player_turn);

    uint64_t occupied = ((state->v | (state->v >> 1)) & GAME_MASK_BOARD_EVEN);
    uint64_t free_cells = GAME_MASK_BOARD_EVEN & ~occupied;

    uint64_t bits = free_cells;
    while (bits) {
        int bit_idx = 63 - __builtin_clzll(bits);
        uint8_t move = (uint8_t)(bit_idx / 2);
        bits &= ~(1ULL << bit_idx);

        GameState next_state = *state;
        game_take_move(settings, &next_state, current_player, move);
        int eval = minmax_recursive(settings, &next_state, depth - 1, alpha, beta, max_player);
        if (eval > max_eval) {
            max_eval = eval;
            best_move = move;
        }
        if (eval > alpha) alpha = eval;
    }

    // Illegal move last - only take it if it is STRICTLY better than legal moves
    GameState next_state = *state;
    game_take_move(settings, &next_state, current_player, ILLEGAL_MOVE);
    int eval = minmax_recursive(settings, &next_state, depth - 1, alpha, beta, max_player);
    if (eval > max_eval) {
        best_move = ILLEGAL_MOVE;
    }

    return best_move;
}
