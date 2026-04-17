#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "game/game.h"
#include "minunit.h"

int tests_run = 0;

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
