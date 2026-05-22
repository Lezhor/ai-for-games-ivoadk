#include "eval_minmax_linear_complex.h"
#include "game/game_internal.h"
#include "utils/lcg.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


static void evaluate_complex(Evaluator* self, const GameSettings* settings, const GameState* state, double* out_scores) {
    (void)settings;
    LinearComplexEvalState* st = (LinearComplexEvalState*)self->state;

    // 1. Calculate features
    double f[LINEAR_COMPLEX_WEIGHT_COUNT];

    // Inverted scores
    f[0] = (double)(GAME_SCORE_TO_WIN - state->p1_score);
    f[1] = (double)(GAME_SCORE_TO_WIN - state->p2_score);
    f[2] = (double)(GAME_SCORE_TO_WIN - state->p3_score);

    // Game finished flag
    int finished = game_finished_condition((GameState*)state);
    f[3] = (double)finished;

    // Tournament scores
    uint8_t t_scores[4] = {0};
    if (finished) {
        get_tournament_scores((GameState*)state, t_scores);
    }
    f[4] = (double)t_scores[1];
    f[5] = (double)t_scores[2];
    f[6] = (double)t_scores[3];

    // Stones
    uint8_t stones[4] = {0};
    uint64_t b = state->board;
    for (int i = 0; i < 19; i++) {
        stones[b & 3]++;
        b >>= 2;
    }
    f[7] = (double)stones[1];
    f[8] = (double)stones[2];
    f[9] = (double)stones[3];

    // Combined stuff
    f[10] = (double)(state->p2_score > state->p3_score ? state->p2_score : state->p3_score);
    f[11] = (double)(state->p2_score + state->p3_score);

    // Active flags
    f[12] = (double)game_is_player_active(state, 1);
    f[13] = (double)game_is_player_active(state, 2);
    f[14] = (double)game_is_player_active(state, 3);

    // Empty cells
    f[15] = (double)stones[0];

    // 2. Weighted sum
    double score = 0.0;
    for (int i = 0; i < LINEAR_COMPLEX_WEIGHT_COUNT; i++) {
        score += st->weights[i] * f[i];
    }

    out_scores[0] = score;
}

#ifdef AGENT_TRAINING
static int complex_train(Evaluator* self, void* training_data) {
    LinearComplexEvalState* st = (LinearComplexEvalState*)self->state;
    EAMutationParams* params = (EAMutationParams*)training_data;

    if (params->template_state) {
        memcpy(st->weights, ((const LinearComplexEvalState*)params->template_state)->weights, sizeof(st->weights));
    }

    for (int i = 0; i < LINEAR_COMPLEX_WEIGHT_COUNT; i++) {
        if (lcg_next_double(params->rng) < params->mutation_rate) {
            double noise = params->use_gaussian ? lcg_next_gaussian(params->rng) : lcg_next_double_range(params->rng, -1.0, 1.0);
            st->weights[i] += noise * params->mutation_scale;
        }
    }
    return 0;
}
#endif

static int complex_save(Evaluator* self, const char* filepath) {
    LinearComplexEvalState* st = (LinearComplexEvalState*)self->state;
    FILE* f = fopen(filepath, "w");
    if (!f) return -1;

    for (int i = 0; i < LINEAR_COMPLEX_WEIGHT_COUNT; i++) {
        fprintf(f, "%f%s", st->weights[i], (i == LINEAR_COMPLEX_WEIGHT_COUNT - 1) ? "" : " ");
    }
    fprintf(f, "\n");
    fclose(f);
    return 0;
}

static int complex_load(Evaluator* self, const char* filepath) {
    LinearComplexEvalState* st = (LinearComplexEvalState*)self->state;
    FILE* f = fopen(filepath, "r");
    if (!f) return -1;

    for (int i = 0; i < LINEAR_COMPLEX_WEIGHT_COUNT; i++) {
        if (fscanf(f, "%lf", &st->weights[i]) != 1) {
            fclose(f);
            return -1;
        }
    }
    fclose(f);
    return 0;
}

static void complex_free(Evaluator* self) {
    if (self->state) free(self->state);
    free(self);
}

Evaluator* evaluator_create_minmax_linear_complex(const char* load_path) {
    Evaluator* eval = malloc(sizeof(Evaluator));
    if (!eval) return NULL;

    LinearComplexEvalState* state = malloc(sizeof(LinearComplexEvalState));
    if (!state) {
        free(eval);
        return NULL;
    }

    // Default heuristic: slightly better weights than random but still needs training
    // We want p1_score to be high, opponents scores low.
    // Since we use inverted scores (SCORE_TO_WIN - score), low is better for me.
    // So f[0] (p1_inverted) should have negative weight.
    // f[1], f[2] (opponents_inverted) should have positive weight.
    for (int i = 0; i < LINEAR_COMPLEX_WEIGHT_COUNT; i++) state->weights[i] = 0.0;
    
    state->weights[0] = -10.0; // p1_inverted: -10 means p1_score increases value
    state->weights[1] = 5.0;   // p2_inverted: 5 means p2_score decreases value
    state->weights[2] = 5.0;   // p3_inverted: 5 means p3_score decreases value
    state->weights[7] = 1.0;   // p1_stones
    state->weights[8] = -0.5;  // p2_stones
    state->weights[9] = -0.5;  // p3_stones

    eval->state = state;
    eval->evaluate = evaluate_complex;
    eval->save = complex_save;
    eval->load = complex_load;
    eval->free = complex_free;

#ifdef AGENT_TRAINING
    eval->train = complex_train;
#endif

    if (load_path) {
        if (complex_load(eval, load_path) != 0) {
            fprintf(stderr, "Warning: Failed to load complex linear weights from %s. Using defaults.\n", load_path);
        }
    }

    return eval;
}
