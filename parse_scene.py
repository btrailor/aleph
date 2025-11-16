#!/usr/bin/env python3
"""
Parse Aleph scene files and dump operator information.
Based on the scene format in apps/bees/src/scene.c and net.c
"""

import struct
import sys

# Operator type names from op.h enum
OP_NAMES = [
    "SWITCH",      # 0
    "ENC",         # 1
    "ADD",         # 2
    "MUL",         # 3
    "GATE",        # 4
    "GRID",        # 5 - eOpMonomeGridClassic
    "MIDINOTE",    # 6
    "ADC",         # 7
    "METRO",       # 8
    "PRESET",      # 9
    "TOG",         # 10
    "ACCUM",       # 11
    "SPLIT",       # 12
    "DIV",         # 13
    "SUB",         # 14
    "TIMER",       # 15
    "RANDOM",      # 16
    "LIST8",       # 17
    "THRESH",      # 18
    "MOD",         # 19
    "BITS",        # 20
    "IS",          # 21
    "LOGIC",       # 22
    "LIST2",       # 23
    "LIFE",        # 24
    "HISTORY",     # 25
    "BIGNUM",      # 26
    "SCREEN",      # 27
    "SPLIT4",      # 28
    "DELAY",       # 29
    "ROUTE",       # 30
    "MIDICC",      # 31
    "MIDIOUTNOTE", # 32
    "LIST16",      # 33
    "STEP",        # 34
    "ROUTE8",      # 35
    "CASCADES",    # 36
    "BARS",        # 37
    "SERIAL",      # 38
    "HID",         # 39
    "WW",          # 40
    "ARC",         # 41
    "FADE",        # 42
    "DIVR",        # 43
    "SHL",         # 44
    "SHR",         # 45
    "CHANGE",      # 46
    "ROUTE16",     # 47
    "BARS8",       # 48
    "MIDIOUTCC",   # 49
    "PARAM",       # 50
    "MEM0D",       # 51
    "MEM1D",       # 52
    "MEM2D",       # 53
    "ITER",        # 54
    "GRIDRAW",     # 55 - eOpMonomeGridRaw
    "MIDICLK",     # 56
    "MAGINC",      # 57
    "KRIA",        # 58
    "HARRY",       # 59
    "POLY",        # 60
    "MIDIPROG",    # 61
    "MIDIOUTCLK",  # 62
]

def parse_scene(filename):
    with open(filename, 'rb') as f:
        data = f.read()
    
    print(f"Scene file: {filename}")
    print(f"Total size: {len(data)} bytes")
    print()
    
    # Scene descriptor appears twice in the file:
    # Once at offset 0 (in file header)
    # Once at offset 56 (start of pickle buffer)
    # The actual pickle data starts after the second descriptor
    
    pos = 0
    
    # Read scene name (24 bytes, not 32!)
    scene_name = data[pos:pos+24].decode('ascii', errors='ignore').rstrip('\x00')
    pos += 24
    print(f"Scene name: '{scene_name}'")
    
    # Read module name (24 bytes)
    module_name = data[pos:pos+24].decode('ascii', errors='ignore').rstrip('\x00')
    pos += 24
    print(f"Module name: '{module_name}'")
    
    # Skip to the pickle buffer (starts at offset 56)
    # The pickle buffer ALSO starts with the scene descriptor:
    # - 24 bytes: scene name
    # - 4 bytes: bees version (min, maj, rev[2])
    # - 24 bytes: module name
    # - 4 bytes: module version (min, maj, rev[2])
    # Total: 56 bytes
    
    pos = 56  # Start of pickle buffer
    pos += 56  # Skip descriptor in pickle buffer
    
    # Now we're at net_unpickle() entry point
    print("\n=== Network Data ===")
    
    # Read operator count (4 bytes, u32)
    op_count = struct.unpack('<I', data[pos:pos+4])[0]
    pos += 4
    print(f"Operator count: {op_count}")
    print()
    
    # Read operators
    operators = []
    for i in range(op_count):
        # Read operator class ID (4 bytes, u32)
        op_id = struct.unpack('<I', data[pos:pos+4])[0]
        pos += 4
        
        op_name = OP_NAMES[op_id] if op_id < len(OP_NAMES) else f"UNKNOWN({op_id})"
        operators.append((i, op_id, op_name))
        print(f"Op[{i:2d}]: {op_name:15s} (id={op_id})")
        
        # Skip unpickle data - this varies by operator type
        # For grid operators, it's: focus(4) + tog(4) + mono(4) + LED buffer(256)
        # For now, we'll just move past a conservative amount
        # This is tricky - we'd need to know each operator's pickle size
        # Let's try to detect grid operators and handle them specially
        
        if op_id == 5:  # GRID (eOpMonomeGridClassic)
            # focus + tog + mono + LED buffer
            pos += 4 + 4 + 4 + 256
            print(f"         ^ Grid operator detected, skipped pickle data")
        elif op_id == 55:  # GRIDRAW
            # Similar pickle size
            pos += 4 + 256  # focus + LED buffer (roughly)
            print(f"         ^ GridRaw operator detected, skipped pickle data")
        # Add more operator types as needed
        # For now, we'll break here since we don't know all pickle sizes
    
    print(f"\n=== Summary ===")
    print(f"Total operators: {op_count}")
    grid_ops = [op for op in operators if op[1] in [5, 55]]
    if grid_ops:
        print(f"Grid operators found:")
        for idx, op_id, name in grid_ops:
            print(f"  - Op[{idx}]: {name}")
    else:
        print("No grid operators found!")
    
    return operators

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 parse_scene.py <scene_file.scn>")
        sys.exit(1)
    
    parse_scene(sys.argv[1])
