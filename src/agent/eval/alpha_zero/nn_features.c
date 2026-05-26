#include "nn_features.h"
#include "game/game_internal.h"

void extract_features(const GameSettings* settings, const GameState* game, float* out_features) {
    int cursor = 0;

    // 1. Board Heights (19 floats)
    // Normalized by dividing by 19.0
    for (int i = 0; i < 19; i++) {
        out_features[cursor++] = (float)settings->board_heights[i] / 19.0f;
    }

    // 2. Board Pieces (3 x 19 floats = 57)
    // One-hot encoding: P1 stones, P2 stones, P3 stones
    uint64_t b = game->board;
    for (int p = 1; p <= 3; p++) {
        uint64_t temp_b = b;
        for (int i = 0; i < 19; i++) {
            out_features[cursor++] = (float)((temp_b & 3) == p);
            temp_b >>= 2;
        }
    }

    // 3. Scores (3 floats)
    // Capped at GAME_SCORE_TO_WIN (12) and normalized
    float s1 = (float)game->p1_score;
    float s2 = (float)game->p2_score;
    float s3 = (float)game->p3_score;
    if (s1 > (float)GAME_SCORE_TO_WIN) s1 = (float)GAME_SCORE_TO_WIN;
    if (s2 > (float)GAME_SCORE_TO_WIN) s2 = (float)GAME_SCORE_TO_WIN;
    if (s3 > (float)GAME_SCORE_TO_WIN) s3 = (float)GAME_SCORE_TO_WIN;

    out_features[cursor++] = s1 / (float)GAME_SCORE_TO_WIN;
    out_features[cursor++] = s2 / (float)GAME_SCORE_TO_WIN;
    out_features[cursor++] = s3 / (float)GAME_SCORE_TO_WIN;

    // 4. Active Flags (3 floats)
    out_features[cursor++] = (float)game_is_player_active(game, 1);
    out_features[cursor++] = (float)game_is_player_active(game, 2);
    out_features[cursor++] = (float)game_is_player_active(game, 3);

    // Total: 19 + 57 + 3 + 3 = 82 features
}
