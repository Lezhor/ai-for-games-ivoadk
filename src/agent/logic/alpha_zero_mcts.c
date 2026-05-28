#include "alpha_zero_mcts.h"
#include "game/game_internal.h"
#include <math.h>
#include <string.h>

void mcts_init_context(MCTSContext* ctx, uint64_t seed, double c_puct) {
    lcg_set_seed(&ctx->rng, seed);
    ctx->c_puct = c_puct;
    mcts_reset_tree(ctx);
}

void mcts_reset_tree(MCTSContext* ctx) {
    ctx->pool_cursor = 0;
}

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

    GameState rotated = *game;
    game_cycle_perspective(&rotated, current_turn, 1);
    AlphaZeroEvaluation az_eval;
    eval->evaluate(eval, settings, &rotated, &az_eval);

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

static int32_t mcts_select_child(MCTSContext* ctx, int32_t node_idx, uint8_t player) {
    MCTSNode* n = &ctx->pool[node_idx];
    int32_t best_child = -1;
    double best_uct = -1e20;

    double sqrt_parent_visits = sqrt((double)n->visits + 1e-9);

    int32_t curr = n->first_child_idx;
    while (curr != -1) {
        MCTSNode* c = &ctx->pool[curr];
        double uct;
        if (c->visits == 0) {
            uct = ctx->c_puct * c->prior_prob * sqrt_parent_visits;
        } else {
            double q = c->value_sum[player - 1] / c->visits;
            double u = ctx->c_puct * c->prior_prob * sqrt((double)n->visits) / (1.0 + (double)c->visits);
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

void mcts_iteration(MCTSContext* ctx, const GameSettings* settings, const GameState* game, AlphaZeroEvaluator* eval) {
    int path_len = 0;
    int32_t curr_idx = 0;
    GameState temp_game = *game;

    if (ctx->pool_cursor == 0) {
        mcts_new_node(ctx, -1, 0xFF, 1.0);
        mcts_expand(ctx, 0, settings, game, eval);
    }

    while (ctx->pool[curr_idx].first_child_idx != -1) {
        ctx->path_buffer[path_len++] = curr_idx;
        uint8_t turn = (uint8_t)temp_game.player_turn;
        curr_idx = mcts_select_child(ctx, curr_idx, turn);
        game_take_move(settings, &temp_game, turn, ctx->pool[curr_idx].move_id);
    }
    ctx->path_buffer[path_len++] = curr_idx;

    double abs_v[3];
    if (!game_finished_condition(&temp_game)) {
        uint8_t turn = (uint8_t)temp_game.player_turn;
        GameState rotated = temp_game;
        game_cycle_perspective(&rotated, turn, 1);

        AlphaZeroEvaluation az_eval;
        eval->evaluate(eval, settings, &rotated, &az_eval);

        // Un-rotate value back to absolute perspective
        // Absolute[(p + turn - 1) % 3] = Rotated[p]
        int offset = (int)turn - 1;
        for (int p = 0; p < 3; p++) {
            abs_v[(p + offset) % 3] = az_eval.value[p];
        }

        mcts_expand(ctx, curr_idx, settings, &temp_game, eval);
    } else {
        uint8_t scores[4];
        get_tournament_scores(&temp_game, scores);
        for (int i = 0; i < 3; i++) abs_v[i] = (double)scores[i+1] / 2.0;
    }

    for (int i = 0; i < path_len; i++) {
        MCTSNode* n = &ctx->pool[ctx->path_buffer[i]];
        n->visits++;
        for (int p = 0; p < 3; p++) n->value_sum[p] += abs_v[p];
    }
}

uint8_t mcts_select_best_move(MCTSContext* ctx) {
    if (ctx->pool_cursor == 0) return ILLEGAL_MOVE;
    MCTSNode* root = &ctx->pool[0];
    uint32_t max_v = 0;
    uint8_t best_move = ILLEGAL_MOVE;

    int32_t curr = root->first_child_idx;
    while (curr != -1) {
        if (ctx->pool[curr].visits > max_v) {
            max_v = ctx->pool[curr].visits;
            best_move = ctx->pool[curr].move_id;
        }
        curr = ctx->pool[curr].sibling_idx;
    }
    return best_move;
}
