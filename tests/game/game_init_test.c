#include <stdio.h>

#include "game/board.h"
#include "game/game_internal.h"
#include "minunit.h"
#include "utils/array_utils.h"

int tests_run = 0;

static int triangle_contains_cell(const int base_triangle[3], int cell_id) {
    return (base_triangle[0] == cell_id || base_triangle[1] == cell_id || base_triangle[2] == cell_id);
}

static char* test_game_init_triangles(void) {

    board_height_t heights[BOARD_SIZE] = {
        18, 5, 2, 8, 1, 14, 3, 17, 9, 0, 11, 4, 15, 6, 10, 13, 7, 12, 16
    };

    // just copied over from game_init_triangles()
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

    uint8_t inverse_map[BOARD_SIZE];
    board_height_calculate_inverse_map(heights, inverse_map, BOARD_SIZE);

    Triangle triangles[BOARD_TRIANGLE_COUNT];
    game_init_triangles(heights, triangles);

    for (int i = 0; i < BOARD_TRIANGLE_COUNT; i++) {
        Triangle triangle = triangles[i];

        int rank_L = triangle.shift_L / 2;
        int rank_M = triangle.shift_M / 2;
        int rank_H = triangle.shift_H / 2;

        printf("triangle: %d, %d, %d\n", rank_H, rank_M, rank_L);

        mu_assert("Triangle ranks not properly sorted: L < M", rank_L < rank_M);
        mu_assert("Triangle ranks not properly sorted: M < H", rank_M < rank_H);

        int real_pos_L = inverse_map[rank_L];
        int real_pos_M = inverse_map[rank_M];
        int real_pos_H = inverse_map[rank_H];

        mu_assert("Cell L not in triangle!", triangle_contains_cell(BASE_TRIANGLES[i], real_pos_L));
        mu_assert("Cell M not in triangle!", triangle_contains_cell(BASE_TRIANGLES[i], real_pos_M));
        mu_assert("Cell H not in triangle!", triangle_contains_cell(BASE_TRIANGLES[i], real_pos_H));

        uint64_t expected_clear_mask = (3ULL << triangle.shift_L) |
                                       (3ULL << triangle.shift_M) |
                                       (3ULL << triangle.shift_H);

        mu_assert("triangle.clear_mask doesn't match", expected_clear_mask == triangle.clear_mask);
    }

    return 0;
}

static char* test_all(void) {
    printf("Running game_init tests...\n");
    mu_run_test(test_game_init_triangles);
    return 0;
}

int main(void) {
    char *result = test_all();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result != 0;
}
