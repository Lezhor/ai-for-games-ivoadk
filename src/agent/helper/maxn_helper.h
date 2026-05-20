#ifndef MAXN_HELPER_H
#define MAXN_HELPER_H

#include "game/game.h"
#include <stdint.h>

/**
 * Score structure for 3 players.
 * Designed to be small enough to be passed in registers (6 bytes).
 */
typedef struct {
    int16_t s[4]; // [0] = finished flag, [1-3] = player scores
} MaxNScore;

/**
 * Searches for the best move using the MaxN algorithm.
 * @param settings Game settings
 * @param state Current game state
 * @param depth Max depth to search
 * @param player_id The player ID (1, 2, or 3) that we are maximizing for at the root.
 * @return The best move index (0-18 or 19 for illegal move).
 */
uint8_t maxn_search(const GameSettings* settings, const GameState* state, int depth, uint8_t player_id);

#endif
