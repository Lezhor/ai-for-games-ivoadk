#include "eval_maxn_hardcoded.h"
#include <stdlib.h>

static void evaluate_maxn_hardcoded(Evaluator* self, const GameSettings* settings, const GameState* state, double* out_scores) {
    (void)self;
    (void)settings;

    // MaxN needs absolute scores for all 3 players
    out_scores[0] = (double)state->p1_score;
    out_scores[1] = (double)state->p2_score;
    out_scores[2] = (double)state->p3_score;
}

static int maxn_hardcoded_save(Evaluator* self, const char* filepath) {
    (void)self; (void)filepath;
    return 0;
}

static int maxn_hardcoded_load(Evaluator* self, const char* filepath) {
    (void)self; (void)filepath;
    return 0;
}

static void maxn_hardcoded_free(Evaluator* self) {
    free(self);
}

Evaluator* evaluator_create_maxn_hardcoded(void) {
    Evaluator* eval = malloc(sizeof(Evaluator));
    if (!eval) return NULL;
    
    eval->state = NULL;
    eval->evaluate = evaluate_maxn_hardcoded;
    eval->save = maxn_hardcoded_save;
    eval->load = maxn_hardcoded_load;
    eval->free = maxn_hardcoded_free;
    
    return eval;
}
