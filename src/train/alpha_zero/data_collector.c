#include "data_collector.h"
#include "agent/eval/alpha_zero/nn_features.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void data_collector_create(DataCollector* dc) {
    dc->record_count = 0;
    dc->record_capacity = 128; 
    dc->records = malloc(sizeof(TurnRecord) * dc->record_capacity);
}

void data_collector_init_game(DataCollector* dc, const GameSettings* settings) {
    dc->settings = *settings;
    dc->record_count = 0;
}

void data_collector_record_turn(DataCollector* dc, const GameState* game, const double* policy) {
    if (dc->record_count >= dc->record_capacity) {
        dc->record_capacity *= 2;
        dc->records = realloc(dc->records, sizeof(TurnRecord) * dc->record_capacity);
    }
    dc->records[dc->record_count].game = *game;
    memcpy(dc->records[dc->record_count].policy, policy, sizeof(double) * 20);
    dc->record_count++;
}

void data_collector_flush_game(DataCollector* dc, const double* final_values, const char* csv_path) {
    FILE* f = fopen(csv_path, "a");
    if (!f) return;

    for (size_t i = 0; i < dc->record_count; i++) {
        GameState game = dc->records[i].game;
        uint8_t current_turn = (uint8_t)((game.v & GAME_MASK_PLAYER_TURN) >> 41);

        // 1. Rotate board so current player is P1
        game_cycle_perspective(&game, current_turn, 1);

        // 2. Extract features (~82 floats)
        float features[FEATURE_COUNT];
        extract_features(&dc->settings, &game, features);

        // 3. Rotate final values to match P1 perspective
        double rotated_values[3];
        int diff = (1 - (int)current_turn + 3) % 3;
        for (int p = 0; p < 3; p++) {
            rotated_values[(p + diff) % 3] = final_values[p];
        }

        // 4. Write to CSV
        // Features
        for (int j = 0; j < FEATURE_COUNT; j++) {
            fprintf(f, "%.4f,", features[j]);
        }
        // Policy (20 floats)
        for (int j = 0; j < 20; j++) {
            fprintf(f, "%.4f,", dc->records[i].policy[j]);
        }
        // Values (3 floats)
        fprintf(f, "%.4f,%.4f,%.4f\n", rotated_values[0], rotated_values[1], rotated_values[2]);
    }

    fclose(f);
}

void data_collector_free(DataCollector* dc) {
    if (dc->records) free(dc->records);
    dc->records = NULL;
    dc->record_count = 0;
    dc->record_capacity = 0;
}
