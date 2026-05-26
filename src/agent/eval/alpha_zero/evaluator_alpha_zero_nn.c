#include "evaluator_alpha_zero.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    // Placeholder for weights
    double* weights;
    int weight_count;
} NNEvalState;

static void az_nn_evaluate(AlphaZeroEvaluator* self, const GameSettings* settings, const GameState* state, AlphaZeroEvaluation* out_eval) {
    (void)self; (void)settings; (void)state;
    // TODO: Implement actual forward pass
    // For now, return zero/uniform as placeholder
    for (int i = 0; i < 3; i++) out_eval->value[i] = 0.0;
    for (int i = 0; i < 20; i++) out_eval->policy[i] = 1.0 / 20.0;
}

static int az_nn_save(AlphaZeroEvaluator* self, const char* filepath) {
    // We don't save the weights here because
    (void)self; (void)filepath;
    return 0;
}

static int az_nn_load(AlphaZeroEvaluator* self, const char* filepath) {
    (void)self; (void)filepath;
    // TODO: Load weights from binary/text file
    printf("NN Loader: Loading weights from %s (Not yet implemented)\n", filepath);
    return 0;
}

static void az_nn_free(AlphaZeroEvaluator* self) {
    if (self->state) free(self->state);
    free(self);
}

AlphaZeroEvaluator* evaluator_create_alpha_zero_nn(const char* model_path) {
    AlphaZeroEvaluator* eval = malloc(sizeof(AlphaZeroEvaluator));
    if (!eval) return NULL;

    NNEvalState* state = malloc(sizeof(NNEvalState));
    if (!state) {
        free(eval);
        return NULL;
    }
    state->weights = NULL;
    state->weight_count = 0;

    eval->state = state;
    eval->evaluate = az_nn_evaluate;
    eval->save = az_nn_save;
    eval->load = az_nn_load;
    eval->free = az_nn_free;

    if (model_path) {
        az_nn_load(eval, model_path);
    }

    return eval;
}
