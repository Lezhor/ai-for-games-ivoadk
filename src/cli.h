#ifndef CLI_H
#define CLI_H

typedef struct {
    const char* host;
    int port;
    const char* agent_name;
} AgentConfig;

AgentConfig parse_args(int argc, char* argv[]);

#endif
