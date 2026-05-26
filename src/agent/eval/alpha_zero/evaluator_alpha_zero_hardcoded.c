#include "evaluator_alpha_zero.h"
#include <stdlib.h>

static void az_hardcoded_evaluate(AlphaZeroEvaluator* self, const GameSettings* settings, const GameState* state, AlphaZeroEvaluation* out_eval) {
    (void)self;
    (void)settings;

    uint8_t scores[4];
    get_tournament_scores((GameState*)state, scores);

    out_eval->value[0] = (double)scores[1] / 2.0;
    out_eval->value[1] = (double)scores[2] / 2.0;
    out_eval->value[2] = (double)scores[3] / 2.0;

    for (int i = 0; i < 20; i++) {
        out_eval->policy[i] = 1.0 / 20.0;
    }
}

static int az_hardcoded_save(AlphaZeroEvaluator* self, const char* filepath) {
    (void)self; (void)filepath;
    return 0; // Nothing to save
}

static int az_hardcoded_load(AlphaZeroEvaluator* self, const char* filepath) {
    (void)self; (void)filepath;
    return 0; // Nothing to load
}

static void az_hardcoded_free(AlphaZeroEvaluator* self) {
    free(self);
}

AlphaZeroEvaluator* evaluator_create_alpha_zero_hardcoded(void) {
    AlphaZeroEvaluator* eval = malloc(sizeof(AlphaZeroEvaluator));
    if (!eval) return NULL;

    eval->state = NULL;
    eval->evaluate = az_hardcoded_evaluate;
    eval->save = az_hardcoded_save;
    eval->load = az_hardcoded_load;
    eval->free = az_hardcoded_free;

    return eval;
}
