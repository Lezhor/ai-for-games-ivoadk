#ifndef NN_FEATURES_H
#define NN_FEATURES_H

#include "game/game.h"

#define FEATURE_COUNT 82

/**
 * Extracts normalized features from the game state into a flat array.
 * Assumes the board state is ALREADY rotated to the current player's perspective.
 */
void extract_features(const GameSettings* settings, const GameState* game, float* out_features);

#endif // NN_FEATURES_H
