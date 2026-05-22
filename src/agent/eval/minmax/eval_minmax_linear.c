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

static void evaluate_linear(Evaluator* self, const GameSettings* settings, const GameState* state, uint8_t perspective_player, double* out_scores) {
    (void)settings;
    LinearEvalState* st = (LinearEvalState*)self->state;

    // Check if game finished and return absolute win/loss scores
    if (game_finished_condition((GameState*)state)) {
        uint8_t win_score_val;
        uint8_t winner = game_get_winner((GameState*)state, &win_score_val);
        if (winner == perspective_player) {
            out_scores[0] = WIN_SCORE + state->p1_score; // Simple bonus for actual score
            return;
        }
        if (winner != 0) {
            out_scores[0] = LOSS_SCORE;
            return;
        }
    }

    // Fast mapping based on turn order (1, 2, or 3)
    // 1 -> 2 -> 3 -> 1
    int my_id   = perspective_player;
    int next_id = (perspective_player % 3) + 1;
    int prev_id = ((perspective_player + 1) % 3) + 1;

    double raw_scores[4] = {0.0, (double)state->p1_score, (double)state->p2_score, (double)state->p3_score};

    out_scores[0] = (st->weights[0] * raw_scores[my_id]) +
                    (st->weights[1] * raw_scores[next_id]) +
                    (st->weights[2] * raw_scores[prev_id]);
}

#ifdef AGENT_TRAINING
static int linear_train(Evaluator* self, void* training_data) {
    LinearEvalState* st = (LinearEvalState*)self->state;
    EAMutationParams* params = (EAMutationParams*)training_data;

    if (params->template_state) {
        memcpy(st->weights, params->template_state->weights, sizeof(st->weights));
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
