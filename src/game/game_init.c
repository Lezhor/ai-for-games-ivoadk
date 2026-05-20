#include "game/board.h"
#include "game/game_internal.h"
#include "utils/array_utils.h"
#include "utils/lcg.h"

void game_init_triangles(board_height_t* board_heights, Triangle* out_triangles) {
    const int BASE_TRIANGLES[BOARD_TRIANGLE_COUNT][3] = {
        { 0,  1,  4}, { 1,  2,  5},
        { 0,  3,  4}, { 1,  4,  5}, { 2,  5,  6},
        { 3,  4,  8}, { 4,  5,  9}, { 5,  6, 10},
        { 3,  7,  8}, { 4,  8,  9}, { 5,  9, 10}, { 6, 10, 11},
        { 7,  8, 12}, { 8,  9, 13}, { 9, 10, 14}, {10, 11, 15},
        { 8, 12, 13}, { 9, 13, 14}, {10, 14, 15},
        {12, 13, 16}, {13, 14, 17}, {14, 15, 18},
        {13, 16, 17}, {14, 17, 18}
    };

    for (int i = 0; i < BOARD_TRIANGLE_COUNT; i++) {

        out_triangles[i].v = 0;

        int arr[3] = {
            board_heights[BASE_TRIANGLES[i][0]],
            board_heights[BASE_TRIANGLES[i][1]],
            board_heights[BASE_TRIANGLES[i][2]]
        };

        // sorting triangle by board heights
        if (arr[0] > arr[1]) { int temp = arr[0]; arr[0] = arr[1]; arr[1] = temp; }
        if (arr[1] > arr[2]) { int temp = arr[1]; arr[1] = arr[2]; arr[2] = temp; }
        if (arr[0] > arr[1]) { int temp = arr[0]; arr[0] = arr[1]; arr[1] = temp; }

        int L_id = arr[0];
        int M_id = arr[1];
        int H_id = arr[2];

        out_triangles[i].shift_L = (uint64_t)((L_id * 2) & 0x3F);
        out_triangles[i].shift_M = (uint64_t)((M_id * 2) & 0x3F);
        out_triangles[i].shift_H = (uint64_t)((H_id * 2) & 0x3F);

        out_triangles[i].clear_mask = (3ULL << out_triangles[i].shift_L)
                                    | (3ULL << out_triangles[i].shift_M)
                                    | (3ULL << out_triangles[i].shift_H);

        // find earliest triangle sharing the high corner
        uint64_t h_mask = 3ULL << out_triangles[i].shift_H;
        int jump_back = i + 1; // jump to next one if no prior triangle found
        for (int j = 0; j < i; j++) {
            if (out_triangles[j].clear_mask & h_mask) {
                jump_back = j;
                break;
            }
        }
        out_triangles[i].jump_back = (uint64_t)(jump_back & 0x1F);
    }
}

void game_init_settings(int32_t seed, GameSettings* out_game_settings) {

    // init board heights
    for (int i = 0; i < BOARD_SIZE; i++) {
        out_game_settings->board_heights[i] = (board_height_t)i;
    }
    lcg_t rng;
    lcg_set_seed(&rng, (uint64_t)seed);
    board_height_array_shuffle(out_game_settings->board_heights, BOARD_SIZE, &rng);
    board_height_calculate_inverse_map(out_game_settings->board_heights, out_game_settings->board_inverse_map, BOARD_SIZE);

    game_init_triangles(out_game_settings->board_heights, out_game_settings->triangles);
}

