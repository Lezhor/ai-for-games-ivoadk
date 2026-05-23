#ifndef TOP_LOGIC_H
#define TOP_LOGIC_H

#include "agent/core/agent.h"

/**
 * Creates an agent that always picks the highest available cell index.
 */
Agent* agent_create_top_picker(void);

#endif // TOP_LOGIC_H
