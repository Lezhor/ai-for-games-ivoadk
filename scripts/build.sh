#!/bin/sh

set -e

help() {
  echo "Usage: ./scripts/build.sh [flags] [preset] [agent] [extra flags]"
  echo ""
  echo "Builds and optionally runs the game agent."
  echo ""
  echo "Flags:"
  echo "  --no-run          Build only, do not run the agent."
  echo "  --build-all       Build all agents, not just the passed one. (Normally only builds one target)"
  echo "  --help, help, -h  Show this help message."
  echo ""
  echo "Arguments:"
  echo "  preset            The CMake preset to use (default: tui-debug)."
  echo "                    Possible presets: gui-debug, gui-release, tui-debug, tui-release, headless-debug, headless-release."
  echo "  agent          The agent to run."
  echo "  extra flags       Any additional flags passed to the simulation executable."
  exit 0
}

run=true
build_all=false

while [ "$#" -gt 0 ]; do
  case "$1" in
    --no-run)
      run=false
      shift
      ;;
    --build-all)
      build_all=true
      shift
      ;;
    --help|help|-h)
      help
      ;;
    -*)
      echo "Unknown flag: $1"
      help
      ;;
    *)
      break
      ;;
  esac
done

preset="${1:-tui-debug}"
agent="${2:-randomizer}"

if [ "$#" -gt 0 ]; then
  # pop the preset argument
  shift
  if [ "$#" -gt 0 ]; then
    # pop the agent argument
    shift
  fi
fi

cmake . --preset ${preset}
if [ "$build_all" = true ]; then
  cmake --build --preset ${preset}
else
  cmake --build --preset ${preset} --target "agent-${agent}"
fi

if [ "$run" = true ]; then
  echo "====================================="
  echo "Running Agent: ${agent} (Preset: ${preset})"
  echo "====================================="
  time ./build/${preset}/agent-${agent} $@
fi
