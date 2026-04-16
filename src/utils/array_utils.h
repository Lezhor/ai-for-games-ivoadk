#ifndef ARRAY_UTILS_H
#define ARRAY_UTILS_H

#include "game/board.h"
#include "utils/lcg.h"
#include <stddef.h>
#include <stdint.h>

void board_height_array_shuffle(board_height_t* array, size_t len, lcg_t* rng);
void board_height_calculate_inverse_map(board_height_t* array, board_height_t* out_inverse_map, size_t len);

#endif // ARRAY_UTILS_H
