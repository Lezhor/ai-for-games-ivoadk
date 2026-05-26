#ifndef CLI_H
#define CLI_H

#include "network/network_client.h"

#include "train/alpha_zero/train_alpha_zero.h"

void parse_args(int argc, char* argv[], AgentConfig* config);
void parse_alpha_zero_args(int argc, char* argv[], TrainAlphaZeroConfig* config);

#endif
