#ifndef ARRAY_UTILS_H
#define ARRAY_UTILS_H

#include "board.h"
#include "utils/lcg.h"
#include <stddef.h>
#include <stdint.h>

void board_height_array_shuffle(board_height_t* array, size_t len, lcg_t* rng);

#endif // ARRAY_UTILS_H
