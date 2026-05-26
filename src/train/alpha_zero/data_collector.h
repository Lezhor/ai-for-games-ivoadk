#ifndef DATA_COLLECTOR_H
#define DATA_COLLECTOR_H

#include "game/game.h"
#include <stddef.h>

#define FEATURE_COUNT 82

typedef struct {
    GameState game;
    double policy[20];
} TurnRecord;

typedef struct {
    GameSettings settings;
    TurnRecord* records;
    size_t record_count;
    size_t record_capacity;
} DataCollector;

void data_collector_create(DataCollector* dc);
void data_collector_init_game(DataCollector* dc, const GameSettings* settings);
void data_collector_record_turn(DataCollector* dc, const GameState* game, const double* policy);
void data_collector_flush_game(DataCollector* dc, const double* final_values, const char* csv_path);
void data_collector_free(DataCollector* dc);

#endif // DATA_COLLECTOR_H
