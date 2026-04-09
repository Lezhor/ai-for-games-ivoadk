#!/bin/sh

# used for Setting up clangd lsp for Neovim.
# Pass a build-preset from CMakeUserPresets.txt
# Script setups build and creates compile_commands.json file.
# This file is then linked to repository root where the lsp will look for it.

# needs to be executed from project root!

set -e

# Default to gui-debug if no argument is provided
preset="${1:-tui-debug}"  # TODO: make gui-debug default again once raylib added

echo "Configuring for preset: ${preset}..."

# 1. Configure only (No build). 
# We explicitly force the export of compile commands just in case.
cmake . --preset "${preset}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# 2. Update the symlink in the project root
# Using -f (force) to overwrite the existing link
ln -sf "build/${preset}/compile_commands.json" .

echo "Done! LSP symlink updated to ${preset}. (Run :LspRestart in Neovim if needed)"
