#ifndef GAME_H
#define GAME_H

#include "network/network_client.h"
#include <stdint.h>

typedef union {
    uint64_t v;
    struct {
        uint64_t board           : 38; // Bits 0-37:  The 19 cells (2 bits each)
        uint64_t p2_active       : 1;  // Bit 38:     Player 2 active flag
        uint64_t p3_active       : 1;  // Bit 39:     Player 3 active flag
        int64_t  p2_score_diff   : 8;  // Bits 40-47: Signed score difference!
        int64_t  p3_score_diff   : 8;  // Bits 48-55: Signed score difference!
        uint64_t reserved        : 8;  // Bits 56-63: Unused, pads to exactly 64 bits
    };
} GameState;

// TODO: probably i don't need these anymore if i use the union?
#define MASK_BOARD (uint64_t)0x00000003FFFFFFFFF
#define MASK_PLAYER2_ACTIVE (uint64_t)0x00000004000000000
#define MASK_PLAYER3_ACTIVE (uint64_t)0x00000008000000000
#define MASK_PLAYER2_SCORE_DIFF (uint64_t)0x00000FF0000000000
#define MASK_PLAYER3_SCORE_DIFF (uint64_t)0x000FF000000000000

uint8_t from_network_player(NetworkClient* client, uint8_t player);
uint8_t to_network_player(NetworkClient* client, uint8_t player);

#endif
