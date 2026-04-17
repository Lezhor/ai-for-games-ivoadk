#include "game/board.h"
#include "network/network_client.h"
#include <assert.h>
#include <stdint.h>
#include "game/game.h"

uint8_t game_network_player_from_net(const NetworkClient* client, uint8_t player) {
    assert(player >= 0 && player <= 2 && "player has to be between 0 and 2 in from_network_player");
    // return (uint8_t)((player + 3 - client->player_number) % 3) + 1;
    (void)client;
    return player + 1;
}

uint8_t game_network_player_to_net(const NetworkClient* client, uint8_t player) {
    assert(player >= 1 && player <= 3 && "player has to be between 1 and 3 in to_network_player");
    // return (uint8_t)(player - 1 + client->player_number) % 3;
    (void)client;
    return player - 1;
}
uint8_t game_network_move_index_from_net(const GameSettings* game_settings, uint8_t move) {
    assert(move >= 0 && move < BOARD_SIZE && "move out of bounds in game_network_move_index_from_net()");
    return game_settings->board_heights[move];
}

uint8_t game_network_move_index_to_net(const GameSettings* game_settings, uint8_t move) {
    // note that move == BOARD_SIZE is allowed for intentionally playing illegal move
    assert(move >= 0 && move <= BOARD_SIZE && "move out of bounds in game_network_move_index_to_net()");
    return move == BOARD_SIZE ? BOARD_SIZE : game_settings->board_inverse_map[move];
}

/**
 * receives move and updates game
 */
int game_network_receive_move(const GameSettings* game_settings, const NetworkClient* client, GameState* game) {
    Move received_move;
    int received_status = network_client_receive_move(client, &received_move);
    if (received_status == 0) {
        // its this players turn!
        return 0;
    }
    received_move.player = game_network_player_from_net(client, received_move.player);
    received_move.index = game_network_move_index_from_net(game_settings, received_move.index);
    game_take_move(game_settings, game, received_move.player, received_move.index);
    return 1;
}

void game_network_send_move(const GameSettings* game_settings, const NetworkClient* client, uint8_t move) {
    assert(move >= 0 && move < BOARD_SIZE);
    move = game_network_move_index_to_net(game_settings, move);
    network_client_send_move(client, move);
}
