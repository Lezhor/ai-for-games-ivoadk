#ifndef TRAIN_ALPHA_ZERO_H
#define TRAIN_ALPHA_ZERO_H

#include <stdint.h>

typedef struct {
    int use_nn;
    const char* model_path;
    const char* past_model_path;
    const char* training_data_output;
    int num_games;
    uint32_t seed;
    int seed_provided;
    int num_mcts_iterations;
    double temperature;
    double c_puct;

    double p_current;
    double p_past;
    double p_minmax;
    double p_random;
    double p_idler;
} TrainAlphaZeroConfig;

void run_alpha_zero_training_loop(const TrainAlphaZeroConfig* config);

#endif // TRAIN_ALPHA_ZERO_H
