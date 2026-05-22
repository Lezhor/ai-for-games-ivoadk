#include "train_core.h"
#include "utils/time_utils.h"
#ifdef AGENT_TRAINING
#include "game/game.h"
#include "agent/eval/minmax/eval_minmax_linear.h"
#include "game/game_internal.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define POPULATION_SIZE 20
#define GENERATIONS 100
#define GAMES_PER_AGENT 5
#define ELITISM_COUNT 4
#define MUTATION_RATE 0.3
#define MUTATION_SCALE 0.2

#ifdef AGENT_TRAINING
typedef struct {
    Agent* agent;
    Evaluator* eval;
    double fitness;
} Individual;

static int compare_individuals(const void* a, const void* b) {
    const Individual* ind_a = (const Individual*)a;
    const Individual* ind_b = (const Individual*)b;
    if (ind_a->fitness < ind_b->fitness) return 1;
    if (ind_a->fitness > ind_b->fitness) return -1;
    return 0;
}

static void run_headless_game(int32_t seed, Individual* p1, Individual* p2, Individual* p3) {
    GameSettings settings;
    game_init_settings(seed, &settings); // Default settings

    GameState game;
    game.v = GAME_STATE_DEFAULT_VALUE;

    Individual* players[4] = {NULL, p1, p2, p3};

    while (!game_finished_condition(&game)) {
        uint8_t current_player = game.player_turn;
        Individual* current_ind = players[current_player];

        // 10s deadline for training games (plenty for depth 4)
        uint64_t deadline = time_get_now_ms() + 10000;
        uint8_t move_idx = current_ind->agent->get_move(current_ind->agent, &settings, &game, current_player, deadline);

        game_take_move(&settings, &game, current_player, move_idx);
    }

    // Assign fitness based on tournament points
    uint8_t t_scores[4];
    get_tournament_scores(&game, t_scores);
    p1->fitness += t_scores[1];
    p2->fitness += t_scores[2];
    p3->fitness += t_scores[3];
}

int run_ea_training_loop(int argc, char* argv[], AgentFactory factory, const char* model_path) {
    (void)argc; (void)argv;
    srand((unsigned int)time(NULL));

    Individual population[POPULATION_SIZE];
    for (int i = 0; i < POPULATION_SIZE; i++) {
        population[i].agent = factory(&population[i].eval);
        population[i].fitness = 0;

        // Initial randomization
        EAMutationParams params = {1.0, 1.0, NULL};
        population[i].eval->train(population[i].eval, &params);
    }

    printf("Starting EA Training: %d generations, population size %d\n", GENERATIONS, POPULATION_SIZE);

    for (int gen = 0; gen < GENERATIONS; gen++) {
        // Reset fitness
        for (int i = 0; i < POPULATION_SIZE; i++) population[i].fitness = 0;

        // Round robin (simplified: just random triplets)
        for (int match = 0; match < POPULATION_SIZE * GAMES_PER_AGENT; match++) {
            int i1 = rand() % POPULATION_SIZE;
            int i2 = rand() % POPULATION_SIZE;
            int i3 = rand() % POPULATION_SIZE;
            if (i1 == i2 || i2 == i3 || i1 == i3) continue;

            run_headless_game((int32_t)rand(), &population[i1], &population[i2], &population[i3]);
        }

        // Sort by fitness
        qsort(population, POPULATION_SIZE, sizeof(Individual), compare_individuals);

        // printf("Gen %d: Best Fitness = %.2f, Weights: ", gen, population[0].fitness);
        // population[0].eval->save(population[0].eval, "/dev/stdout");

        // Evolution
        for (int i = ELITISM_COUNT; i < POPULATION_SIZE; i++) {
            // Pick a parent from the elite
            int parent_idx = rand() % ELITISM_COUNT;

            // Mutate loser towards parent
            EAMutationParams params;
            params.mutation_rate = MUTATION_RATE;
            params.mutation_scale = MUTATION_SCALE;
            params.template_state = population[parent_idx].eval->state;

            population[i].eval->train(population[i].eval, &params);
        }

        // Save best periodically
        if (gen % 10 == 0 || gen == GENERATIONS - 1) {
            population[0].eval->save(population[0].eval, model_path);
        }
    }

    // Cleanup
    for (int i = 0; i < POPULATION_SIZE; i++) {
        population[i].agent->free(population[i].agent);
        population[i].eval->free(population[i].eval);
    }

    return EXIT_SUCCESS;
}
#endif
