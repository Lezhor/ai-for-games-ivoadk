#include "eval_minmax_linear.h"
#include "utils/lcg.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WIN_SCORE 100000.0
#define LOSS_SCORE -100000.0

#ifdef AGENT_TRAINING
static double get_random_double(void) {
    // Simple random double between -1.0 and 1.0 using the system rand() for now
    // In a production EA, you might want a better RNG or Gaussian noise.
    return ((double)rand() / (double)RAND_MAX) * 2.0 - 1.0;
}
#endif

static void evaluate_linear(Evaluator* self, const GameSettings* settings, const GameState* state, double* out_scores) {
    (void)settings;
    LinearEvalState* st = (LinearEvalState*)self->state;

    // Always from Player 1's perspective
    out_scores[0] = (st->weights[0] * (double)state->p1_score) +
                    (st->weights[1] * (double)state->p2_score) +
                    (st->weights[2] * (double)state->p3_score);
}

#ifdef AGENT_TRAINING
static int linear_train(Evaluator* self, void* training_data) {
    LinearEvalState* st = (LinearEvalState*)self->state;
    EAMutationParams* params = (EAMutationParams*)training_data;

    if (params->template_state) {
        memcpy(st->weights, ((const LinearEvalState*)params->template_state)->weights, sizeof(st->weights));
    }

    for (int i = 0; i < 3; i++) {
        if (((double)rand() / (double)RAND_MAX) < params->mutation_rate) {
            st->weights[i] += get_random_double() * params->mutation_scale;
        }
    }
    return 0;
}
#endif

static int linear_save(Evaluator* self, const char* filepath) {
    LinearEvalState* st = (LinearEvalState*)self->state;
    FILE* f = fopen(filepath, "w");
    if (!f) return -1;

    fprintf(f, "%f %f %f\n", st->weights[0], st->weights[1], st->weights[2]);
    fclose(f);
    return 0;
}

static int linear_load(Evaluator* self, const char* filepath) {
    LinearEvalState* st = (LinearEvalState*)self->state;
    FILE* f = fopen(filepath, "r");
    if (!f) return -1;

    if (fscanf(f, "%lf %lf %lf", &st->weights[0], &st->weights[1], &st->weights[2]) != 3) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

static void linear_free(Evaluator* self) {
    if (self->state) free(self->state);
    free(self);
}

Evaluator* evaluator_create_minmax_linear(const char* load_path) {
    Evaluator* eval = malloc(sizeof(Evaluator));
    if (!eval) return NULL;

    LinearEvalState* state = malloc(sizeof(LinearEvalState));
    if (!state) {
        free(eval);
        return NULL;
    }

    // Default heuristic: 2.0 * my_score - 1.0 * opponent1 - 1.0 * opponent2
    state->weights[0] = 2.0;
    state->weights[1] = -1.0;
    state->weights[2] = -1.0;

    eval->state = state;
    eval->evaluate = evaluate_linear;
    eval->save = linear_save;
    eval->load = linear_load;
    eval->free = linear_free;

#ifdef AGENT_TRAINING
    eval->train = linear_train;
#endif

    if (load_path) {
        if (linear_load(eval, load_path) != 0) {
            fprintf(stderr, "Warning: Failed to load linear weights from %s. Using defaults.\n", load_path);
        }
    }

    return eval;
}
