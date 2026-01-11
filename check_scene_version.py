#!/usr/bin/env python3
import sys
import struct

if len(sys.argv) < 2:
    print("Usage: check_scene_version.py <scene_file>")
    sys.exit(1)

filename = sys.argv[1]

with open(filename, 'rb') as f:
    data = f.read()
    
    # At offset 0x50 (80 decimal) in pickle buffer
    # Read 4 bytes: min, maj, rev (16-bit)
    min_byte = data[0x50]
    maj_byte = data[0x51]
    rev_word = struct.unpack('<H', data[0x52:0x54])[0]
    
    print(f"Scene: {filename}")
    print(f"Bees version at 0x50: min={min_byte} (0x{min_byte:02x}), maj={maj_byte} (0x{maj_byte:02x}), rev={rev_word}")
    print(f"Version interpretation: v{maj_byte}.{min_byte}.{rev_word}")
    print(f"Node format: {'128 nodes (v0.4-v0.7)' if maj_byte < 8 else '256 nodes (v0.8+)'}")
