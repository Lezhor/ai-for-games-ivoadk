#!/bin/bash

# Directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RENDERER="$SCRIPT_DIR/render_kanji.swift"

FONT_NAME="${1:-Klee Demibold}"
# Sanitize font name: lowercase, replace spaces with underscores, prepend 'kanji_'
CLEAN_NAME=$(echo "$FONT_NAME" | tr '[:upper:]' '[:lower:]' | tr ' ' '_')
DIR_NAME="kanji_$CLEAN_NAME"
OUTPUT_DIR="$SCRIPT_DIR/../png/$DIR_NAME"

# Ensure output directory exists
mkdir -p "$OUTPUT_DIR"

# Define Kanji entries: "character:name"
declare -a KANJI_ENTRIES=(
    "愛:ai"
    "魅:mi"
    "中:naka"
    "何:nani"
    "忍:ninja"
    "幸:sachi"
    "桜:sakura"
    "友:tomo"
    "禅:zen"
)

echo "Starting batch rendering of Kanji icons using font '$FONT_NAME'..."

for entry in "${KANJI_ENTRIES[@]}"; do
    char="${entry%%:*}"
    name="${entry##*:}"
    
    output_file="$OUTPUT_DIR/kanji_${name}.png"
    
    echo "Rendering '$char' as '$output_file'..."
    swift "$RENDERER" "$char" "$FONT_NAME" "$output_file"
done

echo "Batch rendering complete. Files are in $OUTPUT_DIR"
