#ifndef CLI_H
#define CLI_H

#include "network/network_client.h"

void parse_args(int argc, char* argv[], AgentConfig* config);
void parse_alpha_zero_args(int argc, char* argv[], AgentConfig* config);

#endif
