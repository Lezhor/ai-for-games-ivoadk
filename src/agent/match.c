#include "agent/eval/minmax/eval_minmax_linear_complex.h"
#include "game/game.h"
#include "game/game_internal.h"
#include "agent/logic/random_logic.h"
#include "agent/logic/top_logic.h"
#include "agent/logic/minmax_logic.h"
#include "agent/logic/maxn_logic.h"
#include "agent/logic/alpha_zero_logic.h"
#include "agent/eval/minmax/eval_minmax_hardcoded.h"
#include "agent/eval/minmax/eval_minmax_linear.h"
#include "agent/eval/maxn/eval_maxn_hardcoded.h"
#include "agent/eval/alpha_zero/evaluator_alpha_zero.h"
#include "utils/time_utils.h"
#include "utils/lcg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_POOL_SIZE 64

typedef struct {
    char type[32];
    char model_path[256];
} AgentDef;

typedef struct {
    const char* display_name;
    Agent* agent;
} AgentHandle;

typedef struct {
    char name[128];
    uint64_t tournament_points;
    uint64_t games_played;
} AgentStats;

Agent* create_agent(const char* type, const char* model_path) {
    if (strcmp(type, "random") == 0) {
        return agent_create_random((uint64_t)rand());
    } else if (strcmp(type, "top_picker") == 0) {
        return agent_create_top_picker();
    } else if (strcmp(type, "minmax_hardcoded") == 0) {
        Evaluator* eval = evaluator_create_minmax_hardcoded();
        return agent_create_minmax(eval, 4, false);
    } else if (strcmp(type, "minmax_linear") == 0) {
        const char* path = (model_path[0] != '\0') ? model_path : "models/minmax_linear/best.txt";
        Evaluator* eval = evaluator_create_minmax_linear(path);
        return agent_create_minmax(eval, 4, false);
    } else if (strcmp(type, "minmax_linear_complex") == 0) {
        const char* path = (model_path[0] != '\0') ? model_path : "models/minmax_linear_complex/best.txt";
        Evaluator* eval = evaluator_create_minmax_linear_complex(path);
        return agent_create_minmax(eval, 4, false);
    } else if (strcmp(type, "maxn") == 0) {
        Evaluator* eval = evaluator_create_maxn_hardcoded();
        return agent_create_maxn(eval, 4);
    } else if (strcmp(type, "alpha_zero") == 0) {
        AlphaZeroEvaluator* eval;
        if (model_path[0] != '\0') {
            eval = evaluator_create_alpha_zero_nn(model_path);
        } else {
            eval = evaluator_create_alpha_zero_hardcoded();
        }
        if (!eval) return NULL;
        return agent_create_alpha_zero(eval, (uint64_t)rand(), 1.414, 800);
    }
    return NULL;
}

void run_game(int32_t seed, AgentHandle* players[4], uint8_t out_points[4], int time_limit_ms) {
    GameSettings settings;
    game_init_settings(seed, &settings);

    GameState game;
    game.v = GAME_STATE_DEFAULT_VALUE;

    while (!game_finished_condition(&game)) {
        uint8_t current_player = game.player_turn;
        uint64_t deadline = time_get_now_ms() + (uint64_t)time_limit_ms;
        uint8_t move = players[current_player]->agent->get_move(players[current_player]->agent, &settings, &game, current_player, deadline);
        game_take_move(&settings, &game, current_player, move);
    }

    get_tournament_scores(&game, out_points);
}

int main(int argc, char* argv[]) {
    int num_games = 100;
    int time_limit_ms = 50;
    AgentDef agent_pool[MAX_POOL_SIZE];
    int pool_size = 0;
    char* out_filename = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--games") == 0 && i + 1 < argc) {
            num_games = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--time-limit") == 0 && i + 1 < argc) {
            time_limit_ms = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--agents") == 0 && i + 1 < argc) {
            char* list = strdup(argv[++i]);
            char* token = strtok(list, ",");
            while (token && pool_size < MAX_POOL_SIZE) {
                char* colon = strchr(token, ':');
                if (colon) {
                    *colon = '\0';
                    strncpy(agent_pool[pool_size].type, token, 31);
                    strncpy(agent_pool[pool_size].model_path, colon + 1, 255);
                } else {
                    strncpy(agent_pool[pool_size].type, token, 31);
                    agent_pool[pool_size].model_path[0] = '\0';
                }
                pool_size++;
                token = strtok(NULL, ",");
            }
            free(list);
        } else if (strcmp(argv[i], "--out") == 0 && i + 1 < argc) {
            out_filename = argv[++i];
        }
    }

    if (pool_size < 3) {
        fprintf(stderr, "Error: Need at least 3 agents in the pool.\n");
        fprintf(stderr, "Usage example: --agents alpha_zero:models/alpha_zero/epoch0.bin,random,maxn --time-limit 100\n");
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

    AgentStats stats[MAX_POOL_SIZE];
    int stats_count = 0;

    printf("Starting tournament: %d games\n", num_games);
    printf("Time limit per move: %d ms\n", time_limit_ms);
    printf("Pool size: %d agents\n", pool_size);

    uint64_t start_time = time_get_now_ms();
    for (int g = 0; g < num_games; g++) {
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

        AgentHandle p_handles[4];
        AgentHandle* p_ptrs[4] = {NULL, &p_handles[1], &p_handles[2], &p_handles[3]};
        char display_names[4][128];

        for (int i = 1; i <= 3; i++) {
            AgentDef* def = &agent_pool[idx[i-1]];
            if (def->model_path[0] != '\0') {
                const char* filename = strrchr(def->model_path, '/');
                if (filename) filename++; // Skip the slash
                else filename = def->model_path;
                snprintf(display_names[i], 127, "%s:%s", def->type, filename);
            } else {
                snprintf(display_names[i], 127, "%s", def->type);
            }
            p_handles[i].display_name = display_names[i];
            p_handles[i].agent = create_agent(def->type, def->model_path);
            if (!p_handles[i].agent) {
                fprintf(stderr, "\nError: Failed to create agent %s\n", p_handles[i].display_name);
                return 1;
            }
        }

        uint8_t points[4];
        run_game((int32_t)g, p_ptrs, points, time_limit_ms);

        if (csv) {
            fprintf(csv, "%s,%s,%s,%u,%u,%u\n",
                p_handles[1].display_name, p_handles[2].display_name, p_handles[3].display_name,
                points[1], points[2], points[3]);
        }

        for (int i = 1; i <= 3; i++) {
            int s_idx = -1;
            for (int s = 0; s < stats_count; s++) {
                if (strcmp(stats[s].name, p_handles[i].display_name) == 0) {
                    s_idx = s;
                    break;
                }
            }
            if (s_idx == -1) {
                s_idx = stats_count++;
                strncpy(stats[s_idx].name, p_handles[i].display_name, 127);
                stats[s_idx].tournament_points = 0;
                stats[s_idx].games_played = 0;
            }
            stats[s_idx].tournament_points += points[i];
            stats[s_idx].games_played++;
        }

        for (int i = 1; i <= 3; i++) {
            p_handles[i].agent->free(p_handles[i].agent);
        }

        if ((g + 1) % 10 == 0) { printf("."); fflush(stdout); }
    }
    uint64_t end_time = time_get_now_ms();
    if (csv) fclose(csv);

    // Sort by average points (descending)
    for (int i = 0; i < stats_count - 1; i++) {
        for (int j = 0; j < stats_count - i - 1; j++) {
            double avg_j = (double)stats[j].tournament_points / (double)stats[j].games_played;
            double avg_next = (double)stats[j+1].tournament_points / (double)stats[j+1].games_played;
            if (avg_j < avg_next) {
                AgentStats temp = stats[j];
                stats[j] = stats[j+1];
                stats[j+1] = temp;
            }
        }
    }

    printf("\n\nTournament Summary (Aggregated):\n");
    printf("------------------------------------------------------\n");
    printf("%-35s | %-5s | %-7s\n", "Agent (Type:Model)", "Games", "Avg Pts");
    printf("------------------------------------------------------\n");
    for (int i = 0; i < stats_count; i++) {
        printf("%-35s | %-5llu | %-7.2f\n",
            stats[i].name,
            (unsigned long long)stats[i].games_played,
            (double)stats[i].tournament_points / (double)stats[i].games_played);
    }
    printf("------------------------------------------------------\n");
    printf("Total time: %.2fs (%.2f ms/game)\n",
        (double)(end_time - start_time) / 1000.0,
        (double)(end_time - start_time) / num_games);

    return 0;
}
