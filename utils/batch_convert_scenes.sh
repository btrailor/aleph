#!/bin/bash
# Batch convert all .scn files in a directory to v0.8

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CONVERTER="$SCRIPT_DIR/convert_scenes.sh"

if [ $# -lt 2 ]; then
    echo "Usage: $0 <input_dir> <output_dir>"
    echo "  Converts all .scn files from input_dir to v0.8 and saves to output_dir"
    exit 1
fi

INPUT_DIR="$1"
OUTPUT_DIR="$2"

if [ ! -d "$INPUT_DIR" ]; then
    echo "Error: Input directory not found: $INPUT_DIR"
    exit 1
fi

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Convert all .scn files
for scene in "$INPUT_DIR"/*.scn; do
    if [ -f "$scene" ]; then
        filename=$(basename "$scene")
        output="$OUTPUT_DIR/$filename"
        "$CONVERTER" "$scene" "$output"
    fi
done

echo ""
echo "Conversion complete!"
echo "Original scenes: $INPUT_DIR"
echo "Converted scenes: $OUTPUT_DIR"
