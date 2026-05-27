#include "train_alpha_zero.h"
#include "agent/eval/alpha_zero/evaluator_alpha_zero.h"
#include "agent/logic/alpha_zero_mcts.h"
#include "data_collector.h"
#include "game/game.h"
#include "utils/lcg.h"
#include "utils/time_utils.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static uint8_t train_mcts_get_move(const TrainAlphaZeroConfig* config, MCTSContext* ctx, const GameSettings* settings, const GameState* game, AlphaZeroEvaluator* eval, double* out_policy) {
    mcts_reset_tree(ctx);
    
    for (int i = 0; i < config->num_mcts_iterations; i++) {
        mcts_iteration(ctx, settings, game, eval);
    }

    // Extract Policy with Temperature
    MCTSNode* root = &ctx->pool[0];
    memset(out_policy, 0, sizeof(double) * 20);
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
        out_policy[move] = 1.0;
    } else {
        double r = lcg_next_double(&ctx->rng) * total_transformed_visits;
        double acc = 0;
        curr = root->first_child_idx;
        while (curr != -1) {
            MCTSNode* c = &ctx->pool[curr];
            double val = pow((double)c->visits, 1.0 / config->temperature);
            acc += val;
            out_policy[c->move_id] = val / total_transformed_visits;
            if (acc >= r && move == 19) move = c->move_id;
            curr = c->sibling_idx;
        }
    }

    return move;
}

void run_alpha_zero_training_loop(const TrainAlphaZeroConfig* config) {
    printf("Starting AlphaZero training loop...\n");
    printf("Model path: %s\n", config->model_path ? config->model_path : "None");
    printf("Output path: %s\n", config->training_data_output);
    printf("Iterations: %d, Temperature: %.2f, C-PUCT: %.3f\n", config->num_mcts_iterations, config->temperature, config->c_puct);

    AlphaZeroEvaluator* eval = config->use_nn ?
        evaluator_create_alpha_zero_nn(config->model_path) :
        evaluator_create_alpha_zero_hardcoded();

    MCTSContext ctx;
    mcts_init_context(&ctx, config->seed_provided ? config->seed : (uint64_t)time(NULL), config->c_puct);

    DataCollector dc;
    data_collector_create(&dc);

    uint64_t start_ms = time_get_now_ms();

    int games_played = 0;
    while (config->num_games == -1 || games_played < config->num_games) {
        GameSettings settings;
        game_init_settings((int32_t)lcg_next_int(&ctx.rng), &settings);
        GameState game;
        game.v = GAME_STATE_DEFAULT_VALUE;

        data_collector_init_game(&dc, &settings);

        while (!game_finished_condition(&game)) {
            double policy[20];
            uint8_t move = train_mcts_get_move(config, &ctx, &settings, &game, eval, policy);
            data_collector_record_turn(&dc, &game, policy);
            uint8_t turn = (uint8_t)game.player_turn;
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

    uint64_t end_ms = time_get_now_ms();
    uint64_t total_ms = end_ms - start_ms;
    double total_sec = (double)total_ms / 1000.0;
    double avg_ms = (games_played > 0) ? (double)total_ms / (double)games_played : 0;

    printf("\n\nTraining finished!\n");
    printf("Played %d games in %.2f seconds (approx. %d games/second)\n", games_played, total_sec, (int)(1000.0 / (double)avg_ms));

    data_collector_free(&dc);
    eval->free(eval);
}
