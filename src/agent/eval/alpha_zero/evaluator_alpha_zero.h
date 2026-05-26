#ifndef EVALUATOR_ALPHA_ZERO_H
#define EVALUATOR_ALPHA_ZERO_H

#include "game/game.h"

typedef struct AlphaZeroEvaluation {
    double value[3];   // Win probability for P1, P2, P3
    double policy[20]; // Visit probability for the 19 moves + 1 illegal/pass
} AlphaZeroEvaluation;

typedef struct AlphaZeroEvaluator AlphaZeroEvaluator;

struct AlphaZeroEvaluator {
    void* state;

    /**
     * AlphaZero specific evaluation: returns both a Value vector and a Policy vector.
     */
    void (*evaluate)(AlphaZeroEvaluator* self, const GameSettings* settings, const GameState* game, AlphaZeroEvaluation* out_eval);

    /**
     * Saves the evaluator's state (e.g., weights) to a file.
     * @return 0 on success, non-zero on failure.
     */
    // TODO: i don't think we need save for AlphaZero - cuz we just generate training data
    int (*save)(AlphaZeroEvaluator* self, const char* filepath);

    /**
     * Loads the evaluator's state from a file.
     * @return 0 on success, non-zero on failure.
     */
    int (*load)(AlphaZeroEvaluator* self, const char* filepath);

    /**
     * Frees all resources associated with the evaluator.
     */
    void (*free)(AlphaZeroEvaluator* self);
};

AlphaZeroEvaluator* evaluator_create_alpha_zero_hardcoded(void);
AlphaZeroEvaluator* evaluator_create_alpha_zero_nn(const char* model_path);

#endif // EVALUATOR_ALPHA_ZERO_H
