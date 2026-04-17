#include "game/game.h"
#include <stdio.h>

void game_to_string(const GameState* game, char* out_str) {
    int pos = 0;
    out_str[pos++] = '[';
    for (int i = 0; i < BOARD_SIZE; i++) {
        uint8_t cell = (uint8_t)((game->v >> (i * 2)) & 3);
        char c = '-';
        if (cell == 1) c = '1';
        else if (cell == 2) c = '2';
        else if (cell == 3) c = '3';
        out_str[pos++] = c;
    }
    out_str[pos++] = ']';
    out_str[pos++] = ' ';

    pos += snprintf(out_str + pos, 128 - (size_t)pos, "active={");
    int first = 1; // just for propper comma display :)
    if (game->p1_active) {
        pos += snprintf(out_str + pos, 128 - (size_t)pos, "p1");
        first = 0;
    }
    if (game->p2_active) {
        if (!first) {
            out_str[pos++] = ',';
            out_str[pos++] = ' ';
        }
        pos += snprintf(out_str + pos, 128 - (size_t)pos, "p2");
        first = 0;
    }
    if (game->p3_active) {
        if (!first) {
            out_str[pos++] = ',';
            out_str[pos++] = ' ';
        }
        pos += snprintf(out_str + pos, 128 - (size_t)pos, "p3");
    }
    out_str[pos++] = '}';

    pos += snprintf(out_str + pos, 128 - (size_t)pos, " turn=p%u scores={ p1:%3u, p2:%3u, p3:%3u }",
                   (unsigned int)game->player_turn,
                   (unsigned int)game->p1_score,
                   (unsigned int)game->p2_score,
                   (unsigned int)game->p3_score);
}
