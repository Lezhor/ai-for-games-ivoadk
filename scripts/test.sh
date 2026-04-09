#!/bin/sh

set -e

preset="${1:-headless-debug}"

# pop preset to pass other flags to the test
if [ "$#" -gt 0 ]; then
  shift
fi

cmake --preset "${preset}"
cmake --build --preset "${preset}"

echo "====================================="
echo "Running Tests for preset: ${preset}"
echo "====================================="

ctest --preset "${preset}" $@
