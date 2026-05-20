#include "maxn_helper.h"
#include <limits.h>

/**
 * Selection function f(x, y, z, finished) to compare scores.
 * p: current player's score
 * n: next player's score
 * l: last player's score
 * finished: 1 if game ended, 0 otherwise
 *
 * Returns a scalar value representing the desirability of the score distribution.
 */
static inline int f(int16_t p, int16_t n, int16_t l, int16_t finished, int depth) {
    if (finished) {
        // If finished, check if we won or tied for first
        if (p > n && p > l) return 1000000 + depth + p;
        if (p == n || p == l) {
             // Handle ties if necessary, or just treat as slightly better than a loss
             if (p >= n && p >= l) return 500000 + depth + p;
        }
        return -1000000 - depth + p;
    }
    // Heavily weight our own score, slightly penalize opponents to break ties defensively.
    return (p * 2) - n - l;
}

static MaxNScore evaluate_maxn(const GameState* state) {
    MaxNScore scores;
    scores.s[0] = (int16_t)game_finished_condition((GameState*)state);
    scores.s[1] = (int16_t)state->p1_score;
    scores.s[2] = (int16_t)state->p2_score;
    scores.s[3] = (int16_t)state->p3_score;
    return scores;
}

static MaxNScore maxn_recursive(const GameSettings* settings, GameState* state, int depth) {
    if (depth <= 0 || game_finished_condition(state)) {
        return evaluate_maxn(state);
    }

    uint8_t current_player = (uint8_t)state->player_turn; // 1, 2, or 3
    uint8_t next_player = (uint8_t)((current_player % 3) + 1);
    uint8_t last_player = (uint8_t)((next_player % 3) + 1);

    MaxNScore best_score = {0};
    int best_f = INT_MIN;

    uint64_t occupied = ((state->v | (state->v >> 1)) & GAME_MASK_BOARD_EVEN);
    uint64_t bits = GAME_MASK_BOARD_EVEN & ~occupied;

    // Legal moves
    while (bits) {
        int bit_idx = 63 - __builtin_clzll(bits);
        uint8_t move = (uint8_t)(bit_idx / 2);
        bits &= ~(1ULL << bit_idx);

        GameState next_state = *state;
        game_take_move(settings, &next_state, current_player, move);

        MaxNScore current_eval = maxn_recursive(settings, &next_state, depth - 1);

        int current_f = f(current_eval.s[current_player], current_eval.s[next_player], current_eval.s[last_player], current_eval.s[0], depth);

        // Immediate win pruning - if f suggests a win
        if (current_eval.s[0] && current_f > 1000000) {
            return current_eval;
        }

        if (current_f > best_f) {
            best_f = current_f;
            best_score = current_eval;
        }
    }

    // Illegal move
    GameState illegal_state = *state;
    game_take_move(settings, &illegal_state, current_player, ILLEGAL_MOVE);
    MaxNScore illegal_eval = maxn_recursive(settings, &illegal_state, depth - 1);

    int illegal_f = f(illegal_eval.s[current_player], illegal_eval.s[next_player], illegal_eval.s[last_player], illegal_eval.s[0], depth);
    if (illegal_f > best_f) {
        best_score = illegal_eval;
    }

    return best_score;
}

uint8_t maxn_search(const GameSettings* settings, const GameState* state, int depth, uint8_t player_id) {
    (void)player_id;
    uint8_t best_move = ILLEGAL_MOVE;
    int best_f = INT_MIN;

    uint8_t current_player = (uint8_t)state->player_turn;
    uint8_t next_player = (uint8_t)((current_player % 3) + 1);
    uint8_t last_player = (uint8_t)((next_player % 3) + 1);

    uint64_t occupied = ((state->v | (state->v >> 1)) & GAME_MASK_BOARD_EVEN);
    uint64_t bits = GAME_MASK_BOARD_EVEN & ~occupied;

    while (bits) {
        int bit_idx = 63 - __builtin_clzll(bits);
        uint8_t move = (uint8_t)(bit_idx / 2);
        bits &= ~(1ULL << bit_idx);

        GameState next_state = *state;
        game_take_move(settings, &next_state, current_player, move);

        MaxNScore eval = maxn_recursive(settings, &next_state, depth - 1);

        int current_f = f(eval.s[current_player], eval.s[next_player], eval.s[last_player], eval.s[0], depth);

        // Root win check
        if (eval.s[0] && current_f > 1000000) {
            return move;
        }

        if (current_f > best_f) {
            best_f = current_f;
            best_move = move;
        }
    }

    // Check illegal move at root
    GameState illegal_state = *state;
    game_take_move(settings, &illegal_state, current_player, ILLEGAL_MOVE);
    MaxNScore illegal_eval = maxn_recursive(settings, &illegal_state, depth - 1);
    int illegal_f = f(illegal_eval.s[current_player], illegal_eval.s[next_player], illegal_eval.s[last_player], illegal_eval.s[0], depth);

    if (illegal_f > best_f) {
        best_move = ILLEGAL_MOVE;
    }

    return best_move;
}
