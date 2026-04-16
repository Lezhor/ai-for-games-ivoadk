#include "network/network_client.h"
#include <stdint.h>
#include "game/game.h"

uint8_t from_network_player(NetworkClient* client, uint8_t player) {
    return (uint8_t)((player + 3 - client->player_number) % 3) + 1;
}

uint8_t to_network_player(NetworkClient* client, uint8_t player) {
    return (uint8_t)(player - 1 + client->player_number) % 3;
}
