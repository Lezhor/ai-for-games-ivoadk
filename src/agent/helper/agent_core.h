#ifndef AGENT_CORE_H
#define AGENT_CORE_H

#include "cli.h"
#include "game/game.h"
#include "network/network_client.h"

typedef struct {
    AgentConfig config;
    NetworkClient client;
    GameSettings game_settings;
    GameState game;
} AgentContext;

// getmove callback
typedef uint8_t (*AgentStrategyFn)(AgentContext* ctx, void* strategy_data);

/**
 * Initializes the agent context, connects to the server, and sets up the game state
 * @param ctx Empty context to initialize
 * @param argc CLI args
 * @param argv CLI args
 * @param agent_name Name of the agent
 * @param strategy_description A brief description of the agent's strategy for the print out
 */
void agent_init(AgentContext* ctx, int argc, char* argv[], const char* agent_name, const char* strategy_description);

/**
 * standard agent loop
 * @param ctx agent context
 * @param strategy get_move function
 * @param strategy_data Optional user data to pass to the strategy function.
 */
void agent_loop(AgentContext* ctx, AgentStrategyFn strategy, void* strategy_data);

#endif
