#include "game/board.h"
#include "utils/lcg.h"
#include "utils/array_utils.h"
#include <assert.h>
#include <stdint.h>
#include <stddef.h>

void board_height_array_swap(board_height_t* array, size_t idx1, size_t idx2) {
    board_height_t temp = array[idx1];
    array[idx1] = array[idx2];
    array[idx2] = temp;
}

// Exact Implementation of Java's Collections.shuffle()
void board_height_array_shuffle(board_height_t* array, size_t len, lcg_t* rng) {
    assert(array != NULL && "array null in array_shuffle()");
    assert(rng != NULL && "rng null in array_shuffle()");

    if (len <= 1) {
        return;
    }

    for (size_t i = len; i > 1; i--) {
        board_height_array_swap(array, i - 1, (size_t) lcg_next_int_n(rng, (int32_t)i));
    }
}

void board_height_calculate_inverse_map(board_height_t* array, board_height_t* out_inverse_map, size_t len) {
    for (size_t i = 0; i < len; i++) {
        out_inverse_map[array[i]] = (board_height_t)i;
    }
}
