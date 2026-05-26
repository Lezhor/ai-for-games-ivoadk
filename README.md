# AI for Games - IVOADK

A high-performance C11 client for the IVOADK board game, designed to connect to a Java-based server. This project implements multiple different AI agents capable of playing the game autonomously.

**Tested Environments:**
Built and tested primarily on macOS (Clang) and Ubuntu Linux (GCC) using CMake.

## Quickstart

**1. Initialize Submodules (Required):**
```bash
git submodule update --init --recursive
```

**2. Build and run a single agent:**
The build script automatically builds and runs the target agent. You can pass agent-specific flags (like `--host`, `--port`, or `--name`) directly at the end of the command.
```bash
./scripts/build.sh headless-debug minmax_linear --host 127.0.0.1 --port 22135 --name "MinMax Bot"
```

**3. Run a complete match with 3 agents (spawns Java server and connects the agents):**
```bash
./scripts/match.sh tui-release minmax_linear minmax_linear_complex minmax_hardcoded
```

**Alternative: Run a continuous server loop for extended testing:**
```bash
./scripts/run_server_loop.sh
```

## Core Architecture

### High-Performance Game Engine
The core game engine is built for maximum speed, utilizing bitwise operations to process and evaluate board states efficiently. This low-level approach enables lightning-fast node expansion during game tree searches.

### Networking & Synchronization
The client communicates with the official Java server via a custom protocol. To minimize network payload, the server broadcasts only an initial random seed rather than the full board state. To ensure the C client generates the exact same initial board heights as the Java server, a custom Linear Congruential Generator (LCG) strictly mirroring the exact behavior of `java.util.Random` was implemented from scratch.

### Build System & Automation
The project uses a modular CMake build system supporting optional GUI (Raylib) and TUI (ncurses) interfaces. Development is heavily streamlined through automation scripts located in the `scripts/` directory (e.g., `build.sh`, `test.sh`, `match.sh`), alongside dedicated scripts for generating and converting base64 icon assets.

### Modular Agent Entrypoints
Adding new agents is trivial. The CMake configuration (`add_agent_executable`) links the core simulation engine to a single standalone file in `src/agent/`. This structure isolates agent logic while providing immediate access to the high-performance engine, making experimentation and testing effortless.

## AI Implementation

### MinMax & Evolutionary Algorithm
The primary requirement of the project is fulfilled through a heavily optimized MinMax agent. To accurately evaluate board states at the leaf nodes of the MinMax search tree, the evaluation functions are continuously trained and refined using an evolutionary algorithm.

### AlphaZero (In Progress)
An AlphaZero-inspired agent is currently in development. The architecture is structurally in place, and the agent is currently in the initial data generation phase.

### Testing & Baseline Agents
Several simpler agents are implemented for benchmarking and validation:
- **Randomizer**: Selects moves uniformly at random (only legal moves).
- **Top Picker**: Always chooses the move the cell with the highest board height which is free.
- **Idler**: Immediately sends an illegal move on purpose to disconnect.

## Tournament Benchmarking

For rapid evaluation of agent performance, `src/agent/match.c` provides a standalone tournament runner. It simulates thousands of games locally using the high-performance engine (bypassing the Java server) to provide statistically significant win rates and performance metrics.

**Sample Tournament Output:**
```text
Tournament Summary (Aggregated):
----------------------------------------------------------------------
Agent Type           | Games      | Total Pts  | Avg Pts
----------------------------------------------------------------------
minmax_linear        | 609        | 752        | 1.23
top_picker           | 599        | 664        | 1.11
random               | 577        | 4          | 0.01
minmax_linear_complex | 595        | 839        | 1.41
minmax_hardcoded     | 620        | 675        | 1.09
----------------------------------------------------------------------
Total time: 0.54s (0.54 ms/game)
```

(I cowrote this README.md with Gemini 3 Pro cuz i couldnt be bothered to do it myself)
