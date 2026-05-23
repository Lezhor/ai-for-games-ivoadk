#ifndef EVAL_MINMAX_LINEAR_COMPLEX_H
#define EVAL_MINMAX_LINEAR_COMPLEX_H

#include "agent/eval/evaluator.h"

#define LINEAR_COMPLEX_WEIGHT_COUNT 16

/**
 * Linear evaluation state with complex features.
 * Features (all from Player 1 perspective):
 * 0:  SCORE_TO_WIN - p1_score (inverted p1 score)
 * 1:  SCORE_TO_WIN - p2_score (inverted p2 score)
 * 2:  SCORE_TO_WIN - p3_score (inverted p3 score)
 * 3:  game_finished (1 if finished, 0 otherwise)
 * 4:  tourney_p1 (tournament score if finished, else 0)
 * 5:  tourney_p2
 * 6:  tourney_p3
 * 7:  p1_stones (number of stones on board for p1)
 * 8:  p2_stones
 * 9:  p3_stones
 * 10: max(p2_score, p3_score)
 * 11: p2_score + p3_score
 * 12: p1_active (1 if active, 0 otherwise)
 * 13: p2_active
 * 14: p3_active
 * 15: empty_cells (number of empty cells on board)
 */
typedef struct {
    double weights[LINEAR_COMPLEX_WEIGHT_COUNT];
} LinearComplexEvalState;

#ifdef AGENT_TRAINING
#include "train/train_core.h"
#endif

/**
 * Creates a complex linear evaluator for MinMax.
 * @param load_path Optional path to load weights from.
 * @return A new Evaluator instance.
 */
Evaluator* evaluator_create_minmax_linear_complex(const char* load_path);

#endif // EVAL_MINMAX_LINEAR_COMPLEX_H
