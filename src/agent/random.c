#include <stdio.h>
#include <stdlib.h>
#include "cli.h"

/**
 * minimal test
 * Usage: ./agent <host> <port>
 */
int main(int argc, char *argv[]) {
    AgentConfig config = parse_args(argc, argv);

    printf("--- IVOADK Random Client ---\n");
    printf("Target Host: %s\n", config.host);
    printf("Target Port: %d\n", config.port);
    printf("Agent Name: %s\n", config.agent_name);
    printf("Build system and arguments verified successfully!\n");

    return EXIT_SUCCESS;
}
