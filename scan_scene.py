#!/usr/bin/env python3
"""
Quick scan of scene file for grid operators.
Just looks for the operator enum values in the pickle data.
"""

import struct
import sys

OP_NAMES = {
    0: "SWITCH",
    1: "ENC",
    2: "ADD",
    3: "MUL",
    4: "GATE",
    5: "GRID",        # eOpMonomeGridClassic
    6: "MIDINOTE",
    7: "ADC",
    8: "METRO",
    9: "PRESET",
    34: "STEP",
    55: "GRIDRAW",    # eOpMonomeGridRaw
}

def scan_scene(filename):
    with open(filename, 'rb') as f:
        data = f.read()
    
    print(f"Scene: {filename}")
    print(f"Size: {len(data)} bytes\n")
    
    # Read header
    scene_name = data[0:24].decode('ascii', errors='ignore').rstrip('\x00')
    module_name = data[24:48].decode('ascii', errors='ignore').rstrip('\x00')
    
    print(f"Scene name: '{scene_name}'")
    print(f"Module name: '{module_name}'")
    
    # Read operator count at offset 112
    op_count = struct.unpack('<I', data[112:116])[0]
    print(f"\nOperator count: {op_count}")
    
    if op_count > 128:
        print(f"ERROR: Operator count seems wrong ({op_count} > 128 max)")
        return
    
    # Scan for grid operator IDs in the pickle buffer
    # Start at offset 116 (right after the op count)
    pickle_start = 116
    pickle_end = 56 + 0x40000  # 56 byte header + SCENE_PICKLE_SIZE
    
    print(f"\nScanning for operator type IDs in pickle buffer...")
    print(f"(Offset {pickle_start} to {min(pickle_end, len(data))})\n")
    
    # Look for little-endian u32 values that match our operator IDs
    pos = pickle_start
    found_ops = {}
    
    while pos < min(pickle_end - 4, len(data) - 4):
        val = struct.unpack('<I', data[pos:pos+4])[0]
        if val in OP_NAMES and val <= 62:  # 62 is roughly max op ID
            offset = pos
            op_name = OP_NAMES[val]
            if val not in found_ops:
                found_ops[val] = []
            found_ops[val].append(offset)
            # Don't report every instance, just first few
            if len(found_ops[val]) <= 3:
                print(f"  Offset 0x{offset:04x} ({offset:5d}): {op_name:15s} (id={val})")
        pos += 1
    
    print(f"\n=== Summary ===")
    for op_id in sorted(found_ops.keys()):
        count = len(found_ops[op_id])
        print(f"{OP_NAMES[op_id]:15s} (id={op_id:2d}): found at {count} offsets")
    
    # Check specifically for grid operators
    grid_found = False
    if 5 in found_ops:
        print(f"\n⚠️  GRID operator (id=5) found at {len(found_ops[5])} locations!")
        grid_found = True
    if 55 in found_ops:
        print(f"⚠️  GRIDRAW operator (id=55) found at {len(found_ops[55])} locations!")
        grid_found = True
    if 34 in found_ops:
        print(f"⚠️  STEP operator (id=34) found at {len(found_ops[34])} locations!")
        grid_found = True
        
    if not grid_found:
        print(f"\n✓ No grid/step operators found in this scene")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 scan_scene.py <scene_file.scn>")
        sys.exit(1)
    
    scan_scene(sys.argv[1])
