#ifndef MINMAX_HELPER_H
#define MINMAX_HELPER_H

#include "game/game.h"
#include <stdbool.h>

/**
 * Searches for the best move using Paranoid MinMax with Alpha-Beta pruning for a specific depth.
 * @param settings Game settings
 * @param state Current game state
 * @param depth Max depth to search
 * @param max_player The player ID (1, 2, or 3) that we are maximizing for.
 * @param deadline_ms The absolute time (in ms) by which the search must finish.
 * @param aborted Pointer to a flag that will be set to true if the search is aborted due to timeout.
 * @param node_count Pointer to a counter of visited nodes.
 * @param max_eval_out Optional pointer to store the maximum evaluation score found.
 * @return The best move index (0-18 or 19 for illegal move).
 */
uint8_t minmax_search(const GameSettings* settings, const GameState* state, int depth, uint8_t max_player, uint64_t deadline_ms, bool* aborted, uint64_t* node_count, int* max_eval_out);

/**
 * Iterative deepening MinMax search with time limit.
 * @param settings Game settings
 * @param state Current game state
 * @param max_player The player ID (1, 2, or 3) that we are maximizing for.
 * @param deadline_ms The absolute time (in ms) by which the search must finish.
 * @param out_depth_reached Optional pointer to store the final depth reached.
 * @return The best move index.
 */
uint8_t minmax_search_iterative(const GameSettings* settings, const GameState* state, uint8_t max_player, uint64_t deadline_ms, int* out_depth_reached);

#endif
