#!/bin/sh

set -e

preset="${1:-headless-debug}"
agent="${2:-randomizer}"

if [ "$#" -gt 0 ]; then
  # pop the preset argument
  shift
  if [ "$#" -gt 0 ]; then
    # pop the agent argument
    shift
  fi
fi

echo "====================================="
echo "Running Agent: ${agent} (Preset: ${preset})"
echo "====================================="

time ./build/${preset}/agent-${agent} $@
