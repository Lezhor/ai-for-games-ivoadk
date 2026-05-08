#ifndef MINMAX_HELPER_H
#define MINMAX_HELPER_H

#include "game/game.h"

/**
 * Searches for the best move using Paranoid MinMax with Alpha-Beta pruning.
 * @param settings Game settings
 * @param state Current game state
 * @param depth Max depth to search
 * @param max_player The player ID (1, 2, or 3) that we are maximizing for.
 * @return The best move index (0-18 or 19 for illegal move).
 */
uint8_t minmax_search(const GameSettings* settings, const GameState* state, int depth, uint8_t max_player);

#endif
