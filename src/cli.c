#include "cli.h"
#include <stdlib.h> // for atoi()

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 22135
#define DEFAULT_NAME "Toast Bot"
#define DEFAULT_ICON "./assets/icons/icon_default.b64"

AgentConfig parse_args(int argc, char* argv[]) {
    AgentConfig config;

    // TODO: write parser with dash-flags

    config.host = DEFAULT_HOST;
    config.port = DEFAULT_PORT;
    config.agent_name = DEFAULT_NAME;
    config.icon_path = DEFAULT_ICON;

    if (argc > 1) {
        config.host = argv[1];
    }
    if (argc > 2) {
        config.port = atoi(argv[2]);
    }
    if (argc > 3) {
        config.agent_name = argv[3];
    }
    if (argc > 4) {
        config.icon_path = argv[4];
    }

    return config;
}
