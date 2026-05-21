#include "maxn_logic.h"
#include <stdlib.h>
#include <limits.h>

typedef struct {
    Evaluator* eval;
    int depth;
} MaxNState;

typedef struct {
    int16_t s[4]; // 0: finished, 1: p1, 2: p2, 3: p3
} MaxNScore;

static inline int f_maxn(int16_t p, int16_t n, int16_t l, int16_t finished, int depth) {
    if (finished) {
        if (p > n && p > l) return 1000000 + depth + p;
        if (p >= n && p >= l) return 500000 + depth + p;
        return -1000000 - depth + p;
    }
    return (p * 2) - n - l;
}

static MaxNScore maxn_recursive(const GameSettings* settings, GameState* state, int depth, Evaluator* eval) {
    int finished = game_finished_condition(state);
    if (depth <= 0 || finished) {
        double raw_scores[3];
        eval->evaluate(eval, settings, state, 0, raw_scores);
        MaxNScore res;
        res.s[0] = (int16_t)finished;
        res.s[1] = (int16_t)raw_scores[0];
        res.s[2] = (int16_t)raw_scores[1];
        res.s[3] = (int16_t)raw_scores[2];
        return res;
    }

    uint8_t current_player = (uint8_t)state->player_turn;
    uint8_t next_player = (uint8_t)((current_player % 3) + 1);
    uint8_t last_player = (uint8_t)((next_player % 3) + 1);

    MaxNScore best_score = {0};
    int best_f = INT_MIN;

    uint64_t occupied = ((state->v | (state->v >> 1)) & GAME_MASK_BOARD_EVEN);
    uint64_t bits = GAME_MASK_BOARD_EVEN & ~occupied;

    while (bits) {
        int bit_idx = 63 - __builtin_clzll(bits);
        uint8_t move = (uint8_t)(bit_idx / 2);
        bits &= ~(1ULL << bit_idx);

        GameState next_state = *state;
        game_take_move(settings, &next_state, current_player, move);

        MaxNScore current_eval = maxn_recursive(settings, &next_state, depth - 1, eval);
        int current_f = f_maxn(current_eval.s[current_player], current_eval.s[next_player], current_eval.s[last_player], current_eval.s[0], depth);

        if (current_eval.s[0] && current_f > 1000000) return current_eval;

        if (current_f > best_f) {
            best_f = current_f;
            best_score = current_eval;
        }
    }

    GameState illegal_state = *state;
    game_take_move(settings, &illegal_state, current_player, ILLEGAL_MOVE);
    MaxNScore illegal_eval = maxn_recursive(settings, &illegal_state, depth - 1, eval);
    int illegal_f = f_maxn(illegal_eval.s[current_player], illegal_eval.s[next_player], illegal_eval.s[last_player], illegal_eval.s[0], depth);
    if (illegal_f > best_f) best_score = illegal_eval;

    return best_score;
}

static uint8_t maxn_get_move(Agent* self, const GameSettings* settings, const GameState* game, uint8_t player_id, uint64_t deadline_ms) {
    (void)player_id; (void)deadline_ms;
    MaxNState* internal = (MaxNState*)self->internal_state;

    uint8_t best_move = ILLEGAL_MOVE;
    int best_f = INT_MIN;

    uint8_t current_player = (uint8_t)game->player_turn;
    uint8_t next_player = (uint8_t)((current_player % 3) + 1);
    uint8_t last_player = (uint8_t)((next_player % 3) + 1);

    uint64_t occupied = ((game->v | (game->v >> 1)) & GAME_MASK_BOARD_EVEN);
    uint64_t bits = GAME_MASK_BOARD_EVEN & ~occupied;

    while (bits) {
        int bit_idx = 63 - __builtin_clzll(bits);
        uint8_t move = (uint8_t)(bit_idx / 2);
        bits &= ~(1ULL << bit_idx);

        GameState next_state = *game;
        game_take_move(settings, &next_state, current_player, move);

        MaxNScore eval = maxn_recursive(settings, &next_state, internal->depth - 1, internal->eval);
        int current_f = f_maxn(eval.s[current_player], eval.s[next_player], eval.s[last_player], eval.s[0], internal->depth);

        if (eval.s[0] && current_f > 1000000) return move;

        if (current_f > best_f) {
            best_f = current_f;
            best_move = move;
        }
    }

    GameState illegal_state = *game;
    game_take_move(settings, &illegal_state, current_player, ILLEGAL_MOVE);
    MaxNScore illegal_eval = maxn_recursive(settings, &illegal_state, internal->depth - 1, internal->eval);
    int illegal_f = f_maxn(illegal_eval.s[current_player], illegal_eval.s[next_player], illegal_eval.s[last_player], illegal_eval.s[0], internal->depth);

    if (illegal_f > best_f) best_move = ILLEGAL_MOVE;

    return best_move;
}

static void maxn_free(Agent* self) {
    if (self->internal_state) free(self->internal_state);
    free(self);
}

Agent* agent_create_maxn(Evaluator* eval, int depth) {
    Agent* agent = malloc(sizeof(Agent));
    if (!agent) return NULL;
    MaxNState* state = malloc(sizeof(MaxNState));
    if (!state) { free(agent); return NULL; }
    state->eval = eval;
    state->depth = depth;
    agent->internal_state = state;
    agent->get_move = maxn_get_move;
    agent->free = maxn_free;
    return agent;
}
