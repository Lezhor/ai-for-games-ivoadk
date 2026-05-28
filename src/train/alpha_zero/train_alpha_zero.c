#include "train_alpha_zero.h"
#include "agent/eval/alpha_zero/evaluator_alpha_zero.h"
#include "agent/eval/minmax/eval_minmax_linear_complex.h"
#include "agent/logic/alpha_zero_mcts.h"
#include "agent/logic/random_logic.h"
#include "agent/logic/minmax_logic.h"
#include "agent/logic/idler_logic.h"
#include "data_collector.h"
#include "game/game.h"
#include "utils/lcg.h"
#include "utils/time_utils.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef enum {
    ROLE_CURRENT,
    ROLE_PAST,
    ROLE_MINMAX,
    ROLE_RANDOM,
    ROLE_IDLER
} AgentRole;

static uint8_t train_mcts_get_move(const TrainAlphaZeroConfig* config, MCTSContext* ctx, const GameSettings* settings, const GameState* game, AlphaZeroEvaluator* eval, double* out_policy) {
    mcts_reset_tree(ctx);

    for (int i = 0; i < config->num_mcts_iterations; i++) {
        mcts_iteration(ctx, settings, game, eval);
    }

    // Extract Policy with Temperature
    MCTSNode* root = &ctx->pool[0];
    if (out_policy) memset(out_policy, 0, sizeof(double) * 20);
    double total_transformed_visits = 0;
    int32_t curr = root->first_child_idx;
    while (curr != -1) {
        MCTSNode* c = &ctx->pool[curr];
        if (c->visits > 0) {
            double val = pow((double)c->visits, 1.0 / config->temperature);
            total_transformed_visits += val;
        }
        curr = c->sibling_idx;
    }

    uint8_t move = 19;
    if (config->temperature < 0.01) {
        move = mcts_select_best_move(ctx);
        if (out_policy) out_policy[move] = 1.0;
    } else {
        double r = lcg_next_double(&ctx->rng) * total_transformed_visits;
        double acc = 0;
        curr = root->first_child_idx;
        while (curr != -1) {
            MCTSNode* c = &ctx->pool[curr];
            double val = pow((double)c->visits, 1.0 / config->temperature);
            acc += val;
            if (out_policy) out_policy[c->move_id] = val / total_transformed_visits;
            if (acc >= r && move == 19) move = c->move_id;
            curr = c->sibling_idx;
        }
    }

    return move;
}

void run_alpha_zero_training_loop(const TrainAlphaZeroConfig* config) {
    printf("Starting AlphaZero training loop...\n");
    printf("Model path: %s\n", config->model_path ? config->model_path : "None");
    printf("Past model path: %s\n", config->past_model_path ? config->past_model_path : "None");
    printf("Output path: %s\n", config->training_data_output);
    printf("P_Current: %.2f, P_Past: %.2f, P_MinMax: %.2f, P_Random: %.2f, P_Idler: %.2f\n",
           config->p_current, config->p_past, config->p_minmax, config->p_random, config->p_idler);

    AlphaZeroEvaluator* eval_current = NULL;
    AlphaZeroEvaluator* eval_past = NULL;
    Agent* agent_minmax = NULL;
    Agent* agent_random = NULL;
    Agent* agent_idler = NULL;
    MCTSContext* ctx_current = NULL;
    MCTSContext* ctx_past = NULL;

    // Initialize Evaluators
    eval_current = config->use_nn ?
        evaluator_create_alpha_zero_nn(config->model_path) :
        evaluator_create_alpha_zero_hardcoded();
    
    if (!eval_current) {
        fprintf(stderr, "Failed to create current evaluator.\n");
        return;
    }

    if (config->p_past > 0 && config->past_model_path) {
        eval_past = evaluator_create_alpha_zero_nn(config->past_model_path);
        if (!eval_past) {
            fprintf(stderr, "Warning: Failed to load past model from %s\n", config->past_model_path);
        }
    }

    // Initialize Other Agents
    if (config->p_minmax > 0) {
        Evaluator* m_eval = evaluator_create_minmax_linear_complex("models/minmax_linear_complex/best.txt");
        agent_minmax = agent_create_minmax(m_eval, 5, false);
    }

    if (config->p_random > 0) {
        agent_random = agent_create_random((uint64_t)time(NULL));
    }

    if (config->p_idler > 0) {
        agent_idler = agent_create_idler();
    }

    ctx_current = malloc(sizeof(MCTSContext));
    if (!ctx_current) {
        fprintf(stderr, "Failed to allocate current MCTS context\n");
        goto cleanup;
    }
    mcts_init_context(ctx_current, config->seed_provided ? config->seed : (uint64_t)time(NULL), config->c_puct);

    if (eval_past) {
        ctx_past = malloc(sizeof(MCTSContext));
        if (ctx_past) {
            mcts_init_context(ctx_past, (uint64_t)time(NULL) + 123, config->c_puct);
        }
    }

    DataCollector dc;
    data_collector_create(&dc);

    int games_played = 0;
    while (config->num_games == -1 || games_played < config->num_games) {
        GameSettings settings;
        game_init_settings((int32_t)lcg_next_int(&ctx_current->rng), &settings);
        GameState game;
        game.v = GAME_STATE_DEFAULT_VALUE;

        // Assign Roles for this game
        AgentRole roles[4];
        for (int p = 1; p <= 3; p++) {
            double r = lcg_next_double(&ctx_current->rng);
            if (r < config->p_current) roles[p] = ROLE_CURRENT;
            else if (r < config->p_current + config->p_past && eval_past && ctx_past) roles[p] = ROLE_PAST;
            else if (r < config->p_current + config->p_past + config->p_minmax && agent_minmax) roles[p] = ROLE_MINMAX;
            else if (r < config->p_current + config->p_past + config->p_minmax + config->p_random && agent_random) roles[p] = ROLE_RANDOM;
            else if (r < config->p_current + config->p_past + config->p_minmax + config->p_random + config->p_idler && agent_idler) roles[p] = ROLE_IDLER;
            else roles[p] = ROLE_CURRENT; // Fallback
        }

        data_collector_init_game(&dc, &settings);

        while (!game_finished_condition(&game)) {
            uint8_t turn = (uint8_t)game.player_turn;
            uint8_t move = ILLEGAL_MOVE;

            if (roles[turn] == ROLE_CURRENT) {
                double policy[20];
                move = train_mcts_get_move(config, ctx_current, &settings, &game, eval_current, policy);
                data_collector_record_turn(&dc, &game, policy);
            } else if (roles[turn] == ROLE_PAST) {
                // Use a lower temperature for past versions to make them strong opponents
                TrainAlphaZeroConfig past_config = *config;
                past_config.temperature = 0.1;
                move = train_mcts_get_move(&past_config, ctx_past, &settings, &game, eval_past, NULL);
            } else if (roles[turn] == ROLE_MINMAX) {
                move = agent_minmax->get_move(agent_minmax, &settings, &game, turn, time_get_now_ms() + 100);
            } else if (roles[turn] == ROLE_RANDOM) {
                move = agent_random->get_move(agent_random, &settings, &game, turn, 0);
            } else if (roles[turn] == ROLE_IDLER) {
                move = agent_idler->get_move(agent_idler, &settings, &game, turn, 0);
            }

            game_take_move(&settings, &game, turn, move);
        }

        uint8_t scores[4];
        get_tournament_scores(&game, scores);
        double final_v[3] = { (double)scores[1]/2.0, (double)scores[2]/2.0, (double)scores[3]/2.0 };
        data_collector_flush_game(&dc, final_v, config->training_data_output);

        games_played++;
        if (games_played % 20 == 0) {
            printf(".");
            fflush(stdout);
        }
        if (games_played % 1000 == 0) printf(" [%d games]\n", games_played);
    }

    printf("\nTraining finished!\n");

cleanup:
    data_collector_free(&dc);
    if (eval_current) eval_current->free(eval_current);
    if (eval_past) eval_past->free(eval_past);
    if (agent_minmax) agent_minmax->free(agent_minmax);
    if (agent_random) agent_random->free(agent_random);
    if (agent_idler) agent_idler->free(agent_idler);
    if (ctx_current) free(ctx_current);
    if (ctx_past) free(ctx_past);
}
