#ifndef MAXN_LOGIC_H
#define MAXN_LOGIC_H

#include "agent/core/agent.h"
#include "agent/eval/evaluator.h"

Agent* agent_create_maxn(Evaluator* eval, int depth);

#endif // MAXN_LOGIC_H
