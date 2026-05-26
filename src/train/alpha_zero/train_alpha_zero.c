#include "train_alpha_zero.h"
#include "agent/eval/alpha_zero/evaluator_alpha_zero.h"
#include "data_collector.h"
#include "game/game.h"
#include "utils/lcg.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MCTS_NODE_POOL_SIZE 10000

typedef struct {
    uint32_t visits;
    double value_sum[3];
    double prior_prob;
    int32_t parent_idx;
    int32_t first_child_idx;
    int32_t sibling_idx;
    uint8_t move_id;
} MCTSNode;

typedef struct {
    MCTSNode pool[MCTS_NODE_POOL_SIZE];
    uint32_t pool_cursor;
    int32_t path_buffer[256]; // just to reuse memory
    lcg_t rng;
} MCTSContext;

static int32_t mcts_new_node(MCTSContext* ctx, int32_t parent_idx, uint8_t move_id, double prior) {
    if (ctx->pool_cursor >= MCTS_NODE_POOL_SIZE) return -1;
    int32_t idx = (int32_t)ctx->pool_cursor++;
    MCTSNode* n = &ctx->pool[idx];
    n->visits = 0;
    for (int i = 0; i < 3; i++) n->value_sum[i] = 0.0;
    n->prior_prob = prior;
    n->parent_idx = parent_idx;
    n->first_child_idx = -1;
    n->sibling_idx = -1;
    n->move_id = move_id;
    return idx;
}

static void mcts_expand(MCTSContext* ctx, int32_t node_idx, const GameSettings* settings, const GameState* game, AlphaZeroEvaluator* eval) {
    MCTSNode* n = &ctx->pool[node_idx];
    uint8_t current_turn = (uint8_t)game->player_turn;

    // 1. Evaluate leaf (rotated)
    GameState rotated = *game;
    game_cycle_perspective(&rotated, current_turn, 1);
    AlphaZeroEvaluation az_eval;
    eval->evaluate(eval, settings, &rotated, &az_eval);

    // 2. Expand children
    int32_t prev_child = -1;
    for (uint8_t m = 0; m < 20; m++) {
        if (m != ILLEGAL_MOVE && !game_is_move_valid(game, m)) continue;

        int32_t child_idx = mcts_new_node(ctx, node_idx, m, az_eval.policy[m]);
        if (child_idx == -1) break;
        if (prev_child == -1) n->first_child_idx = child_idx;
        else ctx->pool[prev_child].sibling_idx = child_idx;
        prev_child = child_idx;
    }
}

static int32_t mcts_select_child(const TrainAlphaZeroConfig* config, MCTSContext* ctx, int32_t node_idx, uint8_t player) {
    MCTSNode* n = &ctx->pool[node_idx];
    int32_t best_child = -1;
    double best_uct = -1e20;

    double sqrt_parent_visits = sqrt((double)n->visits + 1e-9);

    int32_t curr = n->first_child_idx;
    while (curr != -1) {
        MCTSNode* c = &ctx->pool[curr];
        double uct;
        if (c->visits == 0) {
            uct = config->c_puct * c->prior_prob * sqrt_parent_visits;
        } else {
            double q = c->value_sum[player - 1] / c->visits;
            double u = config->c_puct * c->prior_prob * sqrt((double)n->visits) / (1.0 + (double)c->visits);
            uct = q + u;
        }

        if (uct > best_uct) {
            best_uct = uct;
            best_child = curr;
        }
        curr = c->sibling_idx;
    }
    return best_child;
}

static void mcts_iteration(const TrainAlphaZeroConfig* config, MCTSContext* ctx, const GameSettings* settings, const GameState* game, AlphaZeroEvaluator* eval) {
    int path_len = 0;
    int32_t curr_idx = 0;
    GameState temp_game = *game;

    // 1. Selection
    while (ctx->pool[curr_idx].first_child_idx != -1) {
        ctx->path_buffer[path_len++] = curr_idx;
        uint8_t turn = (uint8_t)temp_game.player_turn;
        curr_idx = mcts_select_child(config, ctx, curr_idx, turn);
        game_take_move(settings, &temp_game, turn, ctx->pool[curr_idx].move_id);
    }
    ctx->path_buffer[path_len++] = curr_idx;

    // 2. Expansion & Initial Value
    double abs_v[3];
    if (!game_finished_condition(&temp_game)) {
        uint8_t turn = (uint8_t)temp_game.player_turn;
        GameState rotated = temp_game;
        game_cycle_perspective(&rotated, turn, 1);

        AlphaZeroEvaluation az_eval;
        eval->evaluate(eval, settings, &rotated, &az_eval);

        // Un-rotate value back to absolute perspective
        int diff = (1 - (int)turn + 3) % 3;
        for (int p = 0; p < 3; p++) abs_v[(p + diff) % 3] = az_eval.value[p];

        mcts_expand(ctx, curr_idx, settings, &temp_game, eval);
    } else {
        // Anchoring to truth at terminal nodes
        uint8_t scores[4];
        get_tournament_scores(&temp_game, scores);
        for (int i = 0; i < 3; i++) abs_v[i] = (double)scores[i+1] / 2.0;
    }

    // 3. Backpropagation
    for (int i = 0; i < path_len; i++) {
        MCTSNode* n = &ctx->pool[ctx->path_buffer[i]];
        n->visits++;
        for (int p = 0; p < 3; p++) n->value_sum[p] += abs_v[p];
    }
}

static uint8_t mcts_get_move(const TrainAlphaZeroConfig* config, MCTSContext* ctx, const GameSettings* settings, const GameState* game, AlphaZeroEvaluator* eval, double* out_policy) {
    ctx->pool_cursor = 0;
    mcts_new_node(ctx, -1, 0xFF, 1.0); // Root
    mcts_expand(ctx, 0, settings, game, eval);

    for (int i = 0; i < config->num_mcts_iterations; i++) {
        mcts_iteration(config, ctx, settings, game, eval);
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
        // exploit! (max visits)
        uint32_t max_v = 0;
        curr = root->first_child_idx;
        while (curr != -1) {
            if (ctx->pool[curr].visits > max_v) {
                max_v = ctx->pool[curr].visits;
                move = ctx->pool[curr].move_id;
            }
            curr = ctx->pool[curr].sibling_idx;
        }
        // Fill policy one-hot for the CSV
        out_policy[move] = 1.0;
    } else {
        // Stochastic pick based on temperature
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
    if (config->seed_provided) {
        lcg_set_seed(&ctx.rng, config->seed);
    } else {
        lcg_set_seed(&ctx.rng, (uint64_t)time(NULL));
    }

    DataCollector dc;
    data_collector_create(&dc);

    int games_played = 0;
    while (config->num_games == -1 || games_played < config->num_games) {
        GameSettings settings;
        game_init_settings((int32_t)lcg_next_int(&ctx.rng), &settings);
        GameState game;
        game.v = GAME_STATE_DEFAULT_VALUE;

        data_collector_init_game(&dc, &settings);

        while (!game_finished_condition(&game)) {
            double policy[20];
            uint8_t move = mcts_get_move(config, &ctx, &settings, &game, eval, policy);
            data_collector_record_turn(&dc, &game, policy);
            uint8_t turn = (uint8_t)game.player_turn;
            game_take_move(&settings, &game, turn, move);
        }

        uint8_t scores[4];
        get_tournament_scores(&game, scores);
        double final_v[3] = { (double)scores[1]/2.0, (double)scores[2]/2.0, (double)scores[3]/2.0 };
        data_collector_flush_game(&dc, final_v, config->training_data_output);

        games_played++;
        printf("."); fflush(stdout);
        if (games_played % 50 == 0) printf(" [%d games]\n", games_played);
    }

    data_collector_free(&dc);
    eval->free(eval);
}
