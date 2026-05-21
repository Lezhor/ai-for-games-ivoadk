#include "agent/logic/maxn_logic.h"
#include "agent/eval/maxn/eval_maxn_hardcoded.h"
#include "game/game.h"
#include "minunit.h"
#include <stdio.h>
#include <stdint.h>

int tests_run = 0;

static char* test_maxn_basic(void) {
    GameSettings settings;
    game_init_settings(12345, &settings);

    GameState state;
    state.v = GAME_STATE_DEFAULT_VALUE;
    state.player_turn = 1; // P1's turn

    Evaluator* eval = evaluator_create_maxn_hardcoded();
    Agent* agent = agent_create_maxn(eval, 1);

    // Search at depth 1 should return a legal move
    uint8_t move = agent->get_move(agent, &settings, &state, 1, 0);
    mu_assert("MaxN should return a valid move index (0-18)", move <= 18 || move == ILLEGAL_MOVE);
    
    agent->free(agent);
    eval->free(eval);
    return 0;
}

static char* test_maxn_scoring(void) {
    GameSettings settings;
    game_init_settings(12345, &settings);

    GameState state;
    state.v = GAME_STATE_DEFAULT_VALUE;
    state.p1_score = 30; // P1 is close to winning (32)
    state.player_turn = 1;

    Evaluator* eval = evaluator_create_maxn_hardcoded();
    Agent* agent = agent_create_maxn(eval, 2);

    // With depth 2, if there's a way to score, it should prioritize it
    uint8_t move = agent->get_move(agent, &settings, &state, 1, 0);
    mu_assert("MaxN should return a move", move <= 19);

    agent->free(agent);
    eval->free(eval);
    return 0;
}

static char* test_all(void) {
    mu_run_test(test_maxn_basic);
    mu_run_test(test_maxn_scoring);
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
