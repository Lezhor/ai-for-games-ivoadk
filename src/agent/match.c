#include "agent/eval/minmax/eval_minmax_linear_complex.h"
#include "game/game.h"
#include "game/game_internal.h"
#include "agent/logic/random_logic.h"
#include "agent/logic/top_logic.h"
#include "agent/logic/minmax_logic.h"
#include "agent/logic/maxn_logic.h"
#include "agent/eval/minmax/eval_minmax_hardcoded.h"
#include "agent/eval/minmax/eval_minmax_linear.h"
#include "agent/eval/maxn/eval_maxn_hardcoded.h"
#include "utils/time_utils.h"
#include "utils/lcg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_POOL_SIZE 64

typedef struct {
    const char* type_name;
    Agent* agent;
    Evaluator* eval;
} AgentHandle;

typedef struct {
    char name[64];
    uint64_t tournament_points;
    uint64_t games_played;
} AgentStats;

Agent* create_agent(const char* type, Evaluator** out_eval) {
    *out_eval = NULL;
    if (strcmp(type, "random") == 0) {
        return agent_create_random((uint64_t)rand());
    } else if (strcmp(type, "top_picker") == 0) {
        return agent_create_top_picker();
    } else if (strcmp(type, "minmax_hardcoded") == 0) {
        *out_eval = evaluator_create_minmax_hardcoded();
        return agent_create_minmax(*out_eval, 4, false);
    } else if (strcmp(type, "minmax_linear") == 0) {
        *out_eval = evaluator_create_minmax_linear("models/minmax_linear/best.txt");
        return agent_create_minmax(*out_eval, 4, false);
    } else if (strcmp(type, "minmax_linear_complex") == 0) {
        *out_eval = evaluator_create_minmax_linear_complex("models/minmax_linear_complex/best.txt");
        return agent_create_minmax(*out_eval, 4, false);
    } else if (strcmp(type, "maxn") == 0) {
        *out_eval = evaluator_create_maxn_hardcoded();
        return agent_create_maxn(*out_eval, 4);
    }
    return NULL;
}

void run_game(int32_t seed, AgentHandle* players[4], uint8_t out_points[4]) {
    GameSettings settings;
    game_init_settings(seed, &settings);

    GameState game;
    game.v = GAME_STATE_DEFAULT_VALUE;

    while (!game_finished_condition(&game)) {
        uint8_t current_player = game.player_turn;
        // 50ms deadline per move for batch matches
        uint64_t deadline = time_get_now_ms() + 50;
        uint8_t move = players[current_player]->agent->get_move(players[current_player]->agent, &settings, &game, current_player, deadline);
        game_take_move(&settings, &game, current_player, move);
    }

    get_tournament_scores(&game, out_points);
}

int main(int argc, char* argv[]) {
    int num_games = 100;
    char* agent_pool[MAX_POOL_SIZE];
    int pool_size = 0;
    char* out_filename = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--games") == 0 && i + 1 < argc) {
            num_games = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--agents") == 0 && i + 1 < argc) {
            char* list = strdup(argv[++i]);
            char* token = strtok(list, ",");
            while (token && pool_size < MAX_POOL_SIZE) {
                agent_pool[pool_size++] = strdup(token);
                token = strtok(NULL, ",");
            }
        } else if (strcmp(argv[i], "--out") == 0 && i + 1 < argc) {
            out_filename = argv[++i];
        }
    }

    if (pool_size < 3) {
        fprintf(stderr, "Error: Need at least 3 agents in the pool. Use --agents random,maxn,minmax_linear,...\n");
        return 1;
    }

    FILE* csv = NULL;
    if (out_filename) {
        csv = fopen(out_filename, "w");
        if (!csv) {
            perror("Error opening CSV for writing");
            return 1;
        }
        fprintf(csv, "p1_type,p2_type,p3_type,p1_points,p2_points,p3_points\n");
    }

    lcg_t rng;
    lcg_set_seed(&rng, (uint64_t)time(NULL));

    // Stats for final summary
    AgentStats stats[MAX_POOL_SIZE];
    int stats_count = 0;

    printf("Starting tournament: %d games\n", num_games);
    printf("Pool size: %d agents\n", pool_size);
    if (out_filename) printf("Output: %s\n", out_filename);
    else printf("Output: Summary only (no CSV)\n");

    uint64_t start_time = time_get_now_ms();
    for (int g = 0; g < num_games; g++) {
        // Pick 3 unique indices from pool
        int idx[3];
        for (int i = 0; i < 3; i++) {
            bool unique;
            do {
                unique = true;
                idx[i] = lcg_next_int_n(&rng, pool_size);
                for (int j = 0; j < i; j++) {
                    if (idx[i] == idx[j]) unique = false;
                }
            } while (!unique);
        }

        // Create agents
        AgentHandle p_handles[4];
        AgentHandle* p_ptrs[4] = {NULL, &p_handles[1], &p_handles[2], &p_handles[3]};
        for (int i = 1; i <= 3; i++) {
            p_handles[i].type_name = agent_pool[idx[i-1]];
            p_handles[i].agent = create_agent(p_handles[i].type_name, &p_handles[i].eval);
        }

        uint8_t points[4];
        run_game((int32_t)g, p_ptrs, points);

        // Record to CSV
        if (csv) {
            fprintf(csv, "%s,%s,%s,%u,%u,%u\n",
                p_handles[1].type_name, p_handles[2].type_name, p_handles[3].type_name,
                points[1], points[2], points[3]);
        }

        // Aggregate stats
        for (int i = 1; i <= 3; i++) {
            int s_idx = -1;
            for (int s = 0; s < stats_count; s++) {
                if (strcmp(stats[s].name, p_handles[i].type_name) == 0) {
                    s_idx = s;
                    break;
                }
            }
            if (s_idx == -1) {
                s_idx = stats_count++;
                strncpy(stats[s_idx].name, p_handles[i].type_name, 63);
                stats[s_idx].tournament_points = 0;
                stats[s_idx].games_played = 0;
            }
            stats[s_idx].tournament_points += points[i];
            stats[s_idx].games_played++;
        }

        // Cleanup agents
        for (int i = 1; i <= 3; i++) {
            p_handles[i].agent->free(p_handles[i].agent);
            if (p_handles[i].eval) p_handles[i].eval->free(p_handles[i].eval);
        }

        if ((g + 1) % 10 == 0) { printf("."); fflush(stdout); }
    }
    uint64_t end_time = time_get_now_ms();
    if (csv) fclose(csv);

    printf("\n\nTournament Summary (Aggregated):\n");
    printf("----------------------------------------------------------------------\n");
    printf("%-20s | %-10s | %-10s | %-10s\n", "Agent Type", "Games", "Total Pts", "Avg Pts");
    printf("----------------------------------------------------------------------\n");
    for (int i = 0; i < stats_count; i++) {
        printf("%-20s | %-10llu | %-10llu | %-10.2f\n",
            stats[i].name,
            (unsigned long long)stats[i].games_played,
            (unsigned long long)stats[i].tournament_points,
            (double)stats[i].tournament_points / (double)stats[i].games_played);
    }
    printf("----------------------------------------------------------------------\n");
    printf("Total time: %.2fs (%.2f ms/game)\n",
        (double)(end_time - start_time) / 1000.0,
        (double)(end_time - start_time) / num_games);

    for (int i = 0; i < pool_size; i++) free(agent_pool[i]);

    return 0;
}
