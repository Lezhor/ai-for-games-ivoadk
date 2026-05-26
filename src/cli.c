#include "cli.h"
#include <stdlib.h> // for atoi()
#include <string.h>

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 22135
#define DEFAULT_NAME "Toast Bot"
#define DEFAULT_ICON "./assets/icons/icon_default.b64"

void parse_args(int argc, char* argv[], AgentConfig* config) {
    if (config->host == NULL) config->host = DEFAULT_HOST;
    if (config->port == 0) config->port = DEFAULT_PORT;
    if (config->agent_name == NULL) config->agent_name = DEFAULT_NAME;
    if (config->icon_path == NULL) config->icon_path = DEFAULT_ICON;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--host") == 0 && i + 1 < argc) {
            config->host = argv[++i];
        } else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            config->port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--name") == 0 && i + 1 < argc) {
            config->agent_name = argv[++i];
        } else if (strcmp(argv[i], "--icon") == 0 && i + 1 < argc) {
            config->icon_path = argv[++i];
        }
    }
}

void parse_alpha_zero_args(int argc, char* argv[], AgentConfig* config) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-nn") == 0) {
            config->use_nn = 1;
        } else if (strcmp(argv[i], "--model-path") == 0 && i + 1 < argc) {
            config->model_path = argv[++i];
        } else if (strcmp(argv[i], "--training-data-output") == 0 && i + 1 < argc) {
            config->training_data_output = argv[++i];
        }
    }
}
