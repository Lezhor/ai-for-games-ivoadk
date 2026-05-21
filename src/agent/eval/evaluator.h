#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "game/game.h"

typedef struct Evaluator Evaluator;

struct Evaluator {
    void* state; // e.g. weights array, NN params etc.

    /**
     * Computes the evaluation score(s) for the board state.
     * @param perspective_player The player ID from whose perspective we evaluate.
     * @param out_scores Pointer to an array where scores are written.
     *                   e.g. MinMax writes 1 value to out_scores[0].
     *                   e.g. MaxN writes 3 values to out_scores[0..2].
     */
    void (*evaluate)(Evaluator* self, const GameSettings* settings, const GameState* state, uint8_t perspective_player, double* out_scores);

    /**
     * Saves the evaluator's state (e.g., weights) to a file.
     * @return 0 on success, non-zero on failure.
     */
    int (*save)(Evaluator* self, const char* filepath);

    /**
     * Loads the evaluator's state from a file.
     * @return 0 on success, non-zero on failure.
     */
    int (*load)(Evaluator* self, const char* filepath);

    /**
     * Frees all resources associated with the evaluator.
     */
    void (*free)(Evaluator* self);
};

#endif // EVALUATOR_H
