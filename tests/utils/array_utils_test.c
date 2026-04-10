#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#include "minunit.h"
#include "board.h"
#include "utils/lcg.h"
#include "utils/array_utils.h"

int tests_run = 0;

// --------------------------------------------------------
// Helpers
// --------------------------------------------------------

static int arrays_equal(const board_height_t* expected, const board_height_t* actual, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (expected[i] != actual[i]) {
            printf("\nMismatch at index %zu: Expected %d, got %d\n", i, expected[i], actual[i]);
            return 0;
        }
    }
    return 1;
}

// --------------------------------------------------------
// Tests
// --------------------------------------------------------

static char* test_shuffle_length_one(void) {
    lcg_t rng;

    lcg_set_seed(&rng, 12345L);

    board_height_t array[] = { 99 };
    board_height_t expected[] = { 99 };

    board_height_array_shuffle(array, 1, &rng);

    mu_assert("Error: Length 1 array should not change", arrays_equal(expected, array, 1));
    return 0;
}

static char* test_shuffle_all_same(void) {
    lcg_t rng;
    lcg_set_seed(&rng, 6429L);

    board_height_t array[] = { 5, 5, 5, 5, 5 };
    board_height_t expected[] = { 5, 5, 5, 5, 5 };

    board_height_array_shuffle(array, 5, &rng);

    mu_assert("Error: Array with all identical elements was corrupted", arrays_equal(expected, array, 5));
    return 0;
}

static char* test_shuffle_complex_1(void) {
    lcg_t rng;
    lcg_set_seed(&rng, 42L);

    board_height_t array[19] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19
    };

    // Generated with Java's Collections.shuffle(List<>, Random)
    board_height_t expected[19] = {
        19, 5, 6, 9, 2, 7, 13, 10, 14, 18, 11, 3, 15, 12, 16, 17, 1, 4, 8
    };

    board_height_array_shuffle(array, 19, &rng);

    mu_assert("Error: Shuffled array[19] does not match the Java output for Seed 42!", arrays_equal(expected, array, 19));
    return 0;
}

static char* test_shuffle_complex_2(void) {
    lcg_t rng;
    lcg_set_seed(&rng, 957122L);

    board_height_t array[26] = {
        51, 232, 163, 112, 6, 90, 241, 23, 5, 2, 82, 125, 185, 212, 32, 50, 0, 85, 5, 196, 211, 168, 9, 63, 85, 21
    };

    // Generated with Java's Collections.shuffle(List<>, Random)
    board_height_t expected[26] = {
        168, 50, 5, 63, 112, 23, 163, 232, 32, 211, 212, 185, 85, 51, 196, 21, 241, 2, 125, 5, 90, 85, 0, 9, 6, 82
    };

    board_height_array_shuffle(array, 26, &rng);

    mu_assert("Error: Shuffled array[26] does not match the Java output for Seed 957122!", arrays_equal(expected, array, 26));
    return 0;
}


// --------------------------------------------------------
// Test Runner
// --------------------------------------------------------
static char* test_all(void) {
    printf("Running array_shuffle tests...\n");

    mu_run_test(test_shuffle_length_one);
    mu_run_test(test_shuffle_all_same);
    mu_run_test(test_shuffle_complex_1);
    mu_run_test(test_shuffle_complex_2);

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
