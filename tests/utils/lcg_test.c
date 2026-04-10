#include <stddef.h>

#include "minunit.h"

int tests_run = 0;

// Define constants

// Your tests go here
static char* test_example(void) {
    return 0;
}



static char* test_all(void) {
    printf("Running lcg tests...\n");
    mu_run_test(test_example);
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
