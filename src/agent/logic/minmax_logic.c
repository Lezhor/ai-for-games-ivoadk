#include "minmax_logic.h"
#include "game/game.h"
#include "utils/time_utils.h"
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

typedef struct {
    Evaluator* eval;
    int max_depth;
    bool use_iterative_deepening;
} MinMaxState;

static int minmax_recursive(const GameSettings* settings, GameState* state, int depth, int alpha, int beta, uint64_t deadline_ms, bool* aborted, uint64_t* node_count, Evaluator* eval) {
    if (depth <= 0 || game_finished_condition(state)) {
        double out_scores[1] = {0.0};
        eval->evaluate(eval, settings, state, out_scores);

        return (int)out_scores[0];
    }

    (*node_count)++;
    if ((*node_count & 1023) == 0) {
        if (time_get_now_ms() >= deadline_ms) {
            *aborted = true;
            return 0;
        }
    }

    uint8_t current_player = (uint8_t)(state->player_turn);
    int is_max_node = (current_player == 1);

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
            int e = minmax_recursive(settings, &next_state, depth - 1, alpha, beta, deadline_ms, aborted, node_count, eval);
            if (*aborted) return 0;

            if (e > max_eval) max_eval = e;
            if (e > alpha) alpha = e;
            if (beta <= alpha) break;
        }

        // Try illegal move if not pruned
        if (beta > alpha) {
            GameState next_state = *state;
            game_take_move(settings, &next_state, current_player, ILLEGAL_MOVE);
            int e = minmax_recursive(settings, &next_state, depth - 1, alpha, beta, deadline_ms, aborted, node_count, eval);
            if (*aborted) return 0;
            if (e > max_eval) max_eval = e;
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
            int e = minmax_recursive(settings, &next_state, depth - 1, alpha, beta, deadline_ms, aborted, node_count, eval);
            if (*aborted) return 0;

            if (e < min_eval) min_eval = e;
            if (e < beta) beta = e;
            if (beta <= alpha) break;
        }

        if (beta > alpha) {
            GameState next_state = *state;
            game_take_move(settings, &next_state, current_player, ILLEGAL_MOVE);
            int e = minmax_recursive(settings, &next_state, depth - 1, alpha, beta, deadline_ms, aborted, node_count, eval);
            if (*aborted) return 0;
            if (e < min_eval) min_eval = e;
        }

        return min_eval;
    }
}

static uint8_t do_search(const GameSettings* settings, const GameState* state, int depth, uint64_t deadline_ms, bool* aborted, uint64_t* node_count, Evaluator* eval) {
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
        int e = minmax_recursive(settings, &next_state, depth - 1, alpha, beta, deadline_ms, aborted, node_count, eval);
        if (*aborted) return ILLEGAL_MOVE;

        if (e > max_eval) {
            max_eval = e;
            best_move = move;
        }
        if (e > alpha) alpha = e;
    }

    // Always try illegal move
    GameState next_state = *state;
    game_take_move(settings, &next_state, current_player, ILLEGAL_MOVE);
    int e = minmax_recursive(settings, &next_state, depth - 1, alpha, beta, deadline_ms, aborted, node_count, eval);
    if (*aborted) return ILLEGAL_MOVE;

    if (e > max_eval) {
        max_eval = e;
        best_move = ILLEGAL_MOVE;
    }

    return best_move;
}

static uint8_t minmax_get_move(Agent* self, const GameSettings* settings, const GameState* game, uint8_t player_id, uint64_t deadline_ms) {
    MinMaxState* internal = (MinMaxState*)self->internal_state;

    uint8_t best_move_overall = ILLEGAL_MOVE;
    uint64_t node_count = 0;
    int depth_reached = 0;

    GameState cycled_game = *game;
    game_cycle_perspective(&cycled_game, player_id, 1);

    if (internal->use_iterative_deepening) {
        for (int depth = 1; depth <= internal->max_depth; depth++) {
            bool aborted = false;
            uint8_t move = do_search(settings, &cycled_game, depth, deadline_ms, &aborted, &node_count, internal->eval);
            if (aborted) break;
            best_move_overall = move;
            depth_reached = depth;
        }
    } else {
        bool aborted = false;
        best_move_overall = do_search(settings, &cycled_game, internal->max_depth, deadline_ms, &aborted, &node_count, internal->eval);
        depth_reached = internal->max_depth;
    }

#ifdef NDEBUG
    printf("Depth reached: %d, nodes: %llu\n", depth_reached, node_count);
#endif
    (void)depth_reached;

    return best_move_overall;
}

static void minmax_free(Agent* self) {
    if (self->internal_state) {
        free(self->internal_state);
    }
    free(self);
}

Agent* agent_create_minmax(Evaluator* eval, int max_depth, bool use_iterative_deepening) {
    Agent* agent = malloc(sizeof(Agent));
    if (!agent) return NULL;

    MinMaxState* state = malloc(sizeof(MinMaxState));
    if (!state) {
        free(agent);
        return NULL;
    }

    state->eval = eval;
    state->max_depth = max_depth;
    state->use_iterative_deepening = use_iterative_deepening;

    agent->internal_state = state;
    agent->get_move = minmax_get_move;
    agent->free = minmax_free;

    return agent;
}
