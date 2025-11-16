#!/bin/bash
# Convert v0.4/v0.5/v0.7 scenes to v0.8 by changing version bytes
# Version appears in two places: struct descriptor and pickled data

if [ $# -lt 1 ]; then
    echo "Usage: $0 <scene_file.scn> [output_file.scn]"
    echo "  If output file not specified, will overwrite input file"
    exit 1
fi

INPUT="$1"
OUTPUT="${2:-$INPUT}"

if [ ! -f "$INPUT" ]; then
    echo "Error: Input file not found: $INPUT"
    exit 1
fi

# Create a temporary file
TEMP=$(mktemp)
cp "$INPUT" "$TEMP"

# Change version in descriptor struct (offset 0x30-0x33)
# Format: min(1 byte) maj(1 byte) rev(2 bytes little-endian)
# Target: 0x08 0x00 0x00 0x00 = v0.8.0
printf '\x08\x00\x00\x00' | dd of="$TEMP" bs=1 seek=48 count=4 conv=notrunc 2>/dev/null

# Change version in pickled data (offset 0x4C-0x4F)
# Same format: min(1 byte) maj(1 byte) rev(2 bytes little-endian)
printf '\x08\x00\x00\x00' | dd of="$TEMP" bs=1 seek=76 count=4 conv=notrunc 2>/dev/null

# Move temp file to output
mv "$TEMP" "$OUTPUT"

echo "Converted $INPUT -> $OUTPUT (v0.8.0)"
