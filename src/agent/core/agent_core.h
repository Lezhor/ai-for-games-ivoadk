#ifndef AGENT_CORE_H
#define AGENT_CORE_H

#include "agent.h"
#include "cli.h"

typedef struct {
    AgentConfig config;
    NetworkClient client;
    GameSettings game_settings;
    GameState game;
} AgentContext;

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
 * standard agent loop for network play
 * @param ctx agent context
 * @param agent The agent interface to use for moves
 */
void agent_play_loop(AgentContext* ctx, Agent* agent);

#endif
