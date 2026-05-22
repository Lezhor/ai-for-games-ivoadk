#include "eval_minmax_hardcoded.h"
#include <stdlib.h>

#define WIN_SCORE 100000
#define LOSS_SCORE -100000

static void evaluate_hardcoded(Evaluator* self, const GameSettings* settings, const GameState* state, double* out_scores) {
    (void)self;
    (void)settings;

    // always from perspective of p1

    int my_score = (int)state->p1_score;
    int opponent_score_sum = (int)state->p2_score + (int)state->p3_score;

    // Check if game finished and return absolute win/loss scores
    if (game_finished_condition((GameState*)state)) {
        uint8_t win_score_val;
        uint8_t winner = game_get_winner((GameState*)state, &win_score_val);
        if (winner == 1) {
            out_scores[0] = WIN_SCORE + (double)my_score;
            return;
        }
        if (winner != 0) {
            out_scores[0] = LOSS_SCORE + (double)my_score;
            return;
        }
    }

    // Heuristic: Weighted difference
    out_scores[0] = 2.0 * (double)my_score - (double)opponent_score_sum;
}

static int hardcoded_save(Evaluator* self, const char* filepath) {
    (void)self;
    (void)filepath;
    return 0; // Nothing to save for a hardcoded evaluator
}

static int hardcoded_load(Evaluator* self, const char* filepath) {
    (void)self;
    (void)filepath;
    return 0; // Nothing to load for a hardcoded evaluator
}

static void hardcoded_free(Evaluator* self) {
    free(self);
}

Evaluator* evaluator_create_minmax_hardcoded(void) {
    Evaluator* eval = malloc(sizeof(Evaluator));
    if (!eval) return NULL;

    eval->state = NULL;
    eval->evaluate = evaluate_hardcoded;
    eval->save = hardcoded_save;
    eval->load = hardcoded_load;
    eval->free = hardcoded_free;

    return eval;
}
