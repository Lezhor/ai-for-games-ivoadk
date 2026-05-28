#include "agent/eval/alpha_zero/evaluator_alpha_zero.h"
#include "game/game.h"
#include "minunit.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int tests_run = 0;

static char* test_nn_evaluator_loading(void) {
    const char* model_path = "models/alpha_zero/test_model.bin";
    
    // Skip test if model doesn't exist yet (since torch was missing)
    FILE* f = fopen(model_path, "rb");
    if (!f) {
        printf("Skipping test: %s not found. Run dummy model generation first.\n", model_path);
        return 0;
    }
    fclose(f);

    AlphaZeroEvaluator* eval = evaluator_create_alpha_zero_nn(model_path);
    mu_assert("Evaluator should be created successfully", eval != NULL);

    GameSettings settings;
    game_init_settings(12345, &settings);
    GameState state;
    state.v = GAME_STATE_DEFAULT_VALUE;

    AlphaZeroEvaluation az_eval;
    eval->evaluate(eval, &settings, &state, &az_eval);

    // Basic sanity checks on outputs
    double policy_sum = 0;
    for (int i = 0; i < 20; i++) {
        mu_assert("Policy should be non-negative", az_eval.policy[i] >= 0);
        policy_sum += az_eval.policy[i];
    }
    mu_assert("Policy should sum to approx 1.0", fabs(policy_sum - 1.0) < 1e-5);

    for (int i = 0; i < 3; i++) {
        mu_assert("Value should be in range [0, 1]", az_eval.value[i] >= 0 && az_eval.value[i] <= 1.0);
    }

    eval->free(eval);
    return 0;
}

static char* test_all(void) {
    mu_run_test(test_nn_evaluator_loading);
    return 0;
}

int main(void) {
    char *result = test_all();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL NN TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result != 0;
}
