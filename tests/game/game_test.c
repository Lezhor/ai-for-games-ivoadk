#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "game/game.h"
#include "game/game_internal.h"
#include "minunit.h"

int tests_run = 0;

// helper for filling cells
#define CELL_VAL(cell, player) ((uint64_t)(player) << ((cell) * 2))

// helper for writing score values
#define SCORE_VAL(player) (1ULL << (36 + (player) * 7))

typedef struct {
    uint64_t initial_v;
    uint64_t expected_v;
    const char* description;
} TriangleTestCase;

int assert_triangle_apply_correctly(const GameSettings* settings, uint64_t initial_v, uint64_t expected_v) {
    GameState game;
    game.v = initial_v;

    game_apply_triangles(settings, &game);

    return game.v == expected_v;
}

static char* test_apply_triangles(void) {
    // just normal order / idx == board_height
    board_height_t heights[BOARD_SIZE];
    for (int i = 0; i < BOARD_SIZE; i++) heights[i] = (board_height_t)i;

    GameSettings settings;
    game_init_triangles(heights, settings.triangles);

    TriangleTestCase cases[] = {
        {
            .description = "Standard Trigger (Tri 0: L=1, M=2, H=3) -> P3 scores, M moves to H",
            // Tri 0 uses cells {0, 1, 4}
            .initial_v = CELL_VAL(0, 1) | CELL_VAL(1, 2) | CELL_VAL(4, 3),
            .expected_v = CELL_VAL(4, 2) | SCORE_VAL(3)
        },
        {
            .description = "Rejection: L == M (Should not trigger)",
            .initial_v  = CELL_VAL(0, 2) | CELL_VAL(1, 2) | CELL_VAL(4, 3),
            .expected_v = CELL_VAL(0, 2) | CELL_VAL(1, 2) | CELL_VAL(4, 3)
        },
        {
            .description = "Rejection: L == H (Should not trigger)",
            .initial_v  = CELL_VAL(0, 1) | CELL_VAL(1, 2) | CELL_VAL(4, 1),
            .expected_v = CELL_VAL(0, 1) | CELL_VAL(1, 2) | CELL_VAL(4, 1)
        },
        {
            .description = "Valid: M == H (Should trigger!)",
            .initial_v = CELL_VAL(0, 1) | CELL_VAL(1, 2) | CELL_VAL(4, 2) | SCORE_VAL(1) | SCORE_VAL(2),
            .expected_v = CELL_VAL(4, 2) | SCORE_VAL(1) | (SCORE_VAL(2) * 2)
        },
        {
            .description = "Chain Reaction (Tri 0 followed by Tri 6)",
            // Tri 0 {0, 1, 4}. Tri 6 {4, 5, 9}.
            // Tri 0 triggers, dropping a '2' into cell 4.
            // Tri 6 now has {4=2, 5=3, 9=1}, which triggers Tri 6!
            .initial_v = CELL_VAL(0, 1) | CELL_VAL(1, 2) | CELL_VAL(4, 3) |
                         CELL_VAL(5, 3) | CELL_VAL(9, 1),

            // End result:
            // P3 scores (from Tri 0), P1 scores (from Tri 6)
            // Cell 9 gets the '3' from Cell 5. All other cells are empty.
            .expected_v = CELL_VAL(9, 3) | SCORE_VAL(3) | SCORE_VAL(1)
        }
    };

    int num_cases = sizeof(cases) / sizeof(cases[0]);

    for (int i = 0; i < num_cases; i++) {
        mu_assert(cases[i].description,
                  assert_triangle_apply_correctly(&settings, cases[i].initial_v, cases[i].expected_v));
    }

    return 0;
}

static char* test_jump_back(void) {
    board_height_t heights[BOARD_SIZE] = {
        7, 2, 18, 11, 0, 14, 5, 12, 3, 9, 6, 17, 1, 13, 8, 15, 4, 10, 16
    };

    GameSettings settings;
    game_init_triangles(heights, settings.triangles);

    // note that if it can't jump back it goes to the next index
    uint8_t expected[BOARD_TRIANGLE_COUNT] = {
        1, 2, 3, 1, 1, 2, 1, 1, 9, 6, 1, 12, 8, 14, 6, 11, 13, 13, 15, 13, 13, 22, 13, 21
    };

    for (int i = 0; i < BOARD_TRIANGLE_COUNT; i++) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Triangle %d jump_back mismatch. jmp: %d, exp: %d", i, settings.triangles[i].jump_back, expected[i]);
        mu_assert(msg, settings.triangles[i].jump_back == expected[i]);
    }

    return 0;
}

static char* test_game_turn_advance(void) {
    // test normal if-else branches vs. actual lut implementation
    for (int active = 1; active < 8; active++) {
        uint64_t p1 = (active >> 0) & 1;
        uint64_t p2 = (active >> 1) & 1;
        uint64_t p3 = (active >> 2) & 1;

        for (uint8_t turn = 1; turn <= 3; turn++) {

            GameState state = {
                .player_turn=(uint64_t)(turn & 3),
                .p1_active=(uint64_t)(p1 & 1),
                .p2_active=(uint64_t)(p2 & 1),
                .p3_active=(uint64_t)(p3 & 1)
            };

            // // actually we want to test even if current player is inactive - cuz he might have gone inactive but the turn still needs to be advanced
            // if (!game_is_player_active(&state, turn)) continue;

            // Reference implementation / iterate until you find next active player
            uint8_t next = turn;
            for (int i = 0; i < 3; i++) {
                next = (uint8_t)((next % 3) + 1);
                if ((next == 1 && p1) || (next == 2 && p2) || (next == 3 && p3)) {
                    break;
                }
            }

            game_turn_advance(&state);
            mu_assert("game_turn_advance mismatch", state.player_turn == next);
        }
    }
    return 0;
}

static char* test_game_turn_set(void) {
    // test normal if-else branches vs. actual lut implementation
    for (uint8_t turn = 1; turn <= 3; turn++) {
        for (uint8_t target = 1; target <= 3; target++) {

            GameState state = {
                .player_turn=(uint64_t)(turn & 3),
                .p1_active=(uint64_t)1,
                .p2_active=(uint64_t)1,
                .p3_active=(uint64_t)1
            };

            // Reference: inactivate skipped players
            // we skip only if target is different from turn
            int p1 = 1, p2 = 1, p3 = 1;
            uint8_t skip = turn;
            while (skip != target) {
                if (skip == 1) p1 = 0;
                else if (skip == 2) p2 = 0;
                else if (skip == 3) p3 = 0;
                skip = (uint8_t)((skip % 3) + 1);
            }

            game_turn_set(&state, target);
            mu_assert("game_turn_set turn mismatch", state.player_turn == target);
            mu_assert("game_turn_set p1 mismatch", state.p1_active == (uint64_t)p1);
            mu_assert("game_turn_set p2 mismatch", state.p2_active == (uint64_t)p2);
            mu_assert("game_turn_set p3 mismatch", state.p3_active == (uint64_t)p3);
        }
    }
    return 0;
}

static char* test_all(void) {
    printf("Running game tests...\n");
    mu_run_test(test_apply_triangles);
    mu_run_test(test_jump_back);
    mu_run_test(test_game_turn_advance);
    mu_run_test(test_game_turn_set);
    return 0;
}

int main(void) {
    char *result = test_all();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result != 0;
}
