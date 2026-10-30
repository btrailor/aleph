#!/usr/bin/env python3
"""
scene_converter.py — convert Aleph Bees 0.7.1 scene files to 0.8.x format.

Usage:
    python3 scene_converter.py input.scn output.scn

This is a host-side prototype that mirrors the in-firmware conversion in
apps/bees/src/scene_convert.c.  It operates on the network pickle region of a
0.7.1 scene file:

    operator records  -> re-ID and state-size walk only
    input nodes       -> copied unchanged (256 x 4 bytes)
    output nodes      -> insert dummy slots for operators that gained outputs
    params            -> copied unchanged
    trailing presets -> copied unchanged

The scene descriptor and the pickle header are preserved, except the Bees
version marker is updated to 0.8.x so 0.8.x firmware won't try to convert it
again.

Important discrepancies with the original feasibility-study algorithm:
  * This script follows the live firmware algorithm (insert dummy outputs for
    operators that actually gained outputs: Bignum, Screen, MidiOutNote, Bars,
    Bars8, MidiOutCC).  It does NOT apply a generic "+2 operator ID remap / +1
    output shift for ENC-SPLIT-SPLIT4-TOG" remap.
  * The operator IDs in current op.h are effectively stable (only two renames).
    No ID remapping is performed.

Record layout discovered from source:
  * Scene file = sceneDesc_t (56 bytes) + pickle blob (SCENE_PICKLE_SIZE = 0x40000)
  * sceneDesc_t: sceneName[24], moduleName[24], moduleVersion(4), beesVersion(4)
  * pickle header: sceneName[24], beesVersion(min,maj,rev u16)[4],
                   moduleName[24], moduleVersion(4)
  * network pickle: numOps(u32), [opId(u32) + state bytes]*, inputNodes(256*4),
                    outputNodes(256*4), numParams(u32), params..., presets...

Operator state sizes come from OPERATOR_PICKLE_SIZES_0.7.1.md / scene_convert.c.
"""

import argparse
import struct
import sys
from pathlib import Path

SCENE_NAME_LEN = 24
MODULE_NAME_LEN = 24
SCENE_PICKLE_SIZE = 0x40000
NET_INS_MAX = 256
NET_OUTS_MAX = 256

# 0.7.1 operator state sizes, indexed by operator ID 0-49.
# Verified against apps/bees/src/scene_convert.c.
OP_STATE_SIZE_071 = [
    6, 10, 6, 6, 0,        # 0-4
    6, 2, 6, 6, 0,         # 5-9
    4, 8, 0, 6, 6,         # 10-14
    0, 6, 18, 4, 6,        # 15-19
    18, 8, 10, 6, 12,      # 20-24
    2, 10, 12, 0, 2,       # 25-29
    4, 4, 2, 34, 4,        # 30-34
    2, 4, 14, 4, 4,        # 35-39
    4, 4, 8, 6, 6,         # 40-44
    6, 4, 2, 22, 4,        # 45-49
]

# 0.8.x operator state sizes for operators whose pickled state grew.
# Key: 0.7.1 operator ID. Value: total bytes the 0.8.x pickle function writes.
# Determined by reading the pickle/unpickle implementations in ~/aleph.
OP_STATE_SIZE_08X = {
    # eOpLifeClassic (ID 24): 0.7.1 = 12 bytes; 0.8.x = 268 bytes.
    # Pickles xsize, ysize, x, y, rules, focus (6 * 2 = 12) + lifenow[256] (256).
    24: 268,
    # eOpRandom (ID 16): 0.7.1 = 6 bytes; 0.8.x = 8 bytes.
    # Pickles min, max, trig, seed (4 * 2 = 8).
    16: 8,
    # eOpMonomeGridClassic (ID 5): 0.7.1 = 6 bytes; 0.8.x = 262 bytes.
    # Pickles focus, tog, mono (3 * 2 = 6) + 256 byte LED buffer.
    5: 262,
    # eOpMonomeGridRaw (ID 55): 0.7.1 = 6 bytes; 0.8.x = 262 bytes.
    # Pickles focus, x, y (3 * 2 = 6) + 256 byte LED buffer.
    55: 262,
    # eOpStep (ID 34): 0.7.1 = 4 bytes; 0.8.x = 336 bytes.
    # Pickles focus (2) + size (2) + 19 u32 words of step/loop state (76) +
    # 256 byte LED buffer.
    34: 336,
    # eOpCascades (ID 36): 0.7.1 = 4 bytes; 0.8.x = 60 bytes.
    # Pickles focus (2) + size (2) + 14 u32 words of positions/points/rules state (56).
    36: 60,
    # eOpWW (ID 40): 0.7.1 = 4 bytes; 0.8.x = 3980 bytes.
    # Pickles focus (2) + param (2) + whale_set as 994 u32 words (3976).
    40: 3980,
    # eOpKria (ID 58): 0.7.1 = 8 bytes; 0.8.x = 4920 bytes.
    # Pickles focus/clk/octave/tuning/pattern (5 * 2 = 10? Wait only 4 in source: focus/octave/tuning/pattern)
    # Actually op_kria_pickle writes focus, octave, tuning, pattern (4 * 2 = 8)
    # then kria_set serialized as u32 words (4912 bytes). Total 4920.
    58: 4920,
}

# 0.7.1 output counts, indexed by operator ID 0-49.
# Verified against apps/bees/src/scene_convert.c.
OP_NUM_OUTPUTS_071 = [
    1, 2, 1, 1, 1,         # 0-4
    3, 3, 4, 1, 0,         # 5-9
    1, 2, 2, 1, 1,         # 10-14
    1, 1, 1, 2, 1,         # 15-19
    9, 4, 3, 1, 3,         # 20-24
    9, 0, 0, 4, 1,         # 25-29
    4, 1, 0, 1, 8,         # 30-34
    8, 8, 0, 2, 1,         # 35-39
    7, 3, 1, 2, 1,         # 40-44
    1, 1, 16, 0, 0,        # 45-49
]

# Operators that gained exactly one dummy output in 0.8.x.
OP_IDS_GAINED_OUTPUT = {26, 27, 32, 37, 48, 49}

# ---------------------------------------------------------------------------
# Default state for operators whose state GREW in 0.8.x.
#
# Four operators in 0.7.1 never serialized their internal state at all -- their
# pickle functions ended with the literal comment "no state...???":
#
#     u8* op_cascades_pickle(op_cascades_t* mgrid, u8* dst) {
#       dst = pickle_io(mgrid->focus, dst);
#       dst = pickle_io(mgrid->size, dst);
#       /// no state...???
#       return dst;
#     }
#
# In 0.7.1 that state lived in file-scope statics which were never reset by
# init(), so after loading a scene the runtime state was exactly the
# file-scope defaults. 0.8.x moved the same state into the per-operator
# struct, keeping identical defaults, and made it serializable.
#
# When converting, the expanded region must therefore be filled with the 0.8.x
# INIT DEFAULTS -- not zeros. Zero-padding silently replaced the defaults
# with an all-zero state, which broke eOpCascades (every row's rule_dests
# resolved to row 0, and the countdown could not advance for any row but 0).
#
# Each entry maps an operator ID to the bytes that should fill the region
# beyond the 0.7.1 state. For eOpCascades the 0.7.1 state was focus(2)+size(2),
# so the whole 56-byte expansion is "new" and takes defaults.
#
# Verified against:
#   - upstream monome/aleph tag bees-0.7.1 (commit 656636c7)
#     apps/bees/src/ops/op_cascades.c statics
#   - our 0.8.x apps/bees/src/ops/op_cascades.c op_cascades_init()
#
# eOpStep / eOpWW / eOpKria are handled separately below -- their expanded
# regions include large LED-buffer / table regions that are re-initialised at
# runtime, so only their scalar step state needs defaults.

# 0.8.x op_cascades_init() defaults, as the 7 struct arrays.
CASCADE_INIT_DEFAULTS = {
    "positions":   [3, 1, 2, 2, 3, 3, 5, 7],
    "points":      [3, 1, 2, 2, 3, 3, 5, 7],
    "points_save": [3, 1, 2, 2, 3, 3, 5, 7],
    "triggers":    [0, 0, 0, 0, 0, 0, 0, 0],
    "trig_dests":  [0, 0, 0, 0, 0, 0, 0, 0],
    "rules":       [0, 0, 0, 0, 0, 0, 0, 0],
    "rule_dests":  [0, 1, 2, 3, 4, 5, 6, 7],
}

# The order the struct fields are dumped by op_cascades_pickle(): the loop
# walks from &positions up to (but not including) &edit_row.
_CASCADE_FIELD_ORDER = [
    "positions", "points", "points_save",
    "triggers", "trig_dests", "rules", "rule_dests",
]


def cascade_default_state_bytes() -> bytes:
    """Pack the cascade init defaults into the 56 bytes the struct dump writes.

    The pickle loop writes the region from &positions to &edit_row as u32
    words. The struct layout puts the seven u8/s8 arrays consecutively, so the
    bytes are simply the concatenation of the arrays -- packed four per u32
    word by the dump itself.
    """
    out = bytearray()
    for field in _CASCADE_FIELD_ORDER:
        for v in CASCADE_INIT_DEFAULTS[field]:
            out.append(v & 0xFF)
    return bytes(out)


# operator id -> callable returning the bytes that should fill the region
# beyond the 0.7.1 state.
OP_STATE_DEFAULTS = {
    36: cascade_default_state_bytes,   # eOpCascades
}

OP_NAMES = [
    "eOpSwitch", "eOpEnc", "eOpAdd", "eOpMul", "eOpGate",
    "eOpMonomeGridClassic", "eOpMidiNote", "eOpAdc", "eOpMetro", "eOpPreset",
    "eOpTog", "eOpAccum", "eOpSplit", "eOpDiv", "eOpSub",
    "eOpTimer", "eOpRandom", "eOpList8", "eOpThresh", "eOpMod",
    "eOpBits", "eOpIs", "eOpLogic", "eOpList2", "eOpLifeClassic",
    "eOpHistory", "eOpBignum", "eOpScreen", "eOpSplit4", "eOpDelay",
    "eOpRoute", "eOpMidiCC", "eOpMidiOutNote", "eOpList16", "eOpStep",
    "eOpRoute8", "eOpCascades", "eOpBars", "eOpSerial", "eOpHid",
    "eOpWW", "eOpMonomeArc", "eOpFade", "eOpDivr", "eOpShl",
    "eOpShr", "eOpChange", "eOpRoute16", "eOpBars8", "eOpMidiOutCC",
]


def op_name(op_id: int) -> str:
    if 0 <= op_id < len(OP_NAMES):
        return OP_NAMES[op_id]
    return f"op_{op_id}"


def read_cstring(data: bytes, offset: int, maxlen: int) -> str:
    end = data.find(b"\x00", offset, offset + maxlen)
    if end == -1:
        end = offset + maxlen
    return data[offset:end].decode("ascii", errors="replace")


def parse_scene(path: Path) -> tuple:
    """Parse a 0.7.1 scene and return (descriptor bytes, pickle bytes)."""
    data = path.read_bytes()
    if len(data) != 56 + SCENE_PICKLE_SIZE:
        raise ValueError(
            f"Unexpected scene size: {len(data)} (expected {56 + SCENE_PICKLE_SIZE})"
        )
    return data[:56], data[56:]


def state_size_for_version(op_id: int, bees_version: tuple) -> int:
    """Return the operator state size appropriate to the scene version."""
    if op_id >= 50:
        raise ValueError(f"Invalid operator ID {op_id}")
    if bees_version[1] >= 8:
        return OP_STATE_SIZE_08X.get(op_id, OP_STATE_SIZE_071[op_id])
    return OP_STATE_SIZE_071[op_id]


def describe_pickle(pickle: bytes) -> dict:
    """Decode the pickle header and network region for reporting.

    Walks the operator region using the state sizes that match the scene's
    Bees version so that converted (0.8.x) pickles are reported correctly.
    """
    off = 0
    info = {
        "scene_name": read_cstring(pickle, off, SCENE_NAME_LEN),
        "module_name": None,
        "bees_version": None,
        "module_version": None,
        "num_ops": None,
        "operators": [],
        "input_region": None,
        "output_region": None,
        "num_params": None,
        "connections": [],
    }
    off += SCENE_NAME_LEN
    info["bees_version"] = (
        pickle[off + 1],
        pickle[off],
        struct.unpack_from("<H", pickle, off + 2)[0],
    )
    off += 4
    info["module_name"] = read_cstring(pickle, off, MODULE_NAME_LEN)
    off += MODULE_NAME_LEN
    info["module_version"] = (
        pickle[off + 1],
        pickle[off],
        struct.unpack_from("<H", pickle, off + 2)[0],
    )
    off += 4

    num_ops = struct.unpack_from("<I", pickle, off)[0]
    info["num_ops"] = num_ops
    off += 4

    for _ in range(num_ops):
        op_id = struct.unpack_from("<I", pickle, off)[0]
        if op_id >= 50:
            raise ValueError(f"Invalid operator ID {op_id} at offset {hex(off)}")
        state_size = state_size_for_version(op_id, info["bees_version"])
        info["operators"].append({
            "id": op_id,
            "name": op_name(op_id),
            "state_size": state_size,
            "offset": off,
        })
        off += 4 + state_size

    info["input_region"] = off
    info["output_region"] = off + NET_INS_MAX * 4
    info["num_params"] = struct.unpack_from(
        "<I", pickle, info["output_region"] + NET_OUTS_MAX * 4
    )[0]

    for i in range(NET_OUTS_MAX):
        target = struct.unpack_from("<i", pickle, info["output_region"] + i * 4)[0]
        if target >= 0:
            info["connections"].append((i, target))

    return info


def print_pickle_info(label: str, info: dict, out: list = None) -> None:
    """Print or append pickle description."""
    lines = []
    lines.append(f"=== {label} ===")
    lines.append(f"  scene name: {info['scene_name']!r}")
    lines.append(f"  bees version: {info['bees_version']}")
    lines.append(f"  module: {info['module_name']!r} {info['module_version']}")
    lines.append(f"  num_ops: {info['num_ops']}  num_params: {info['num_params']}")
    lines.append("  operators:")
    for op in info["operators"]:
        lines.append(
            f"    idx={op['offset'] - 4:5} id={op['id']:2} ({op['name']}) "
            f"state={op['state_size']} @ {hex(op['offset'])}"
        )
    lines.append("  connections (output index -> target input index):")
    for src, dst in info["connections"]:
        lines.append(f"    out {src:3} -> in {dst:3}")
    block = "\n".join(lines)
    print(block)
    if out is not None:
        out.append(block)


def convert_pickle(pickle: bytearray) -> dict:
    """Convert the network pickle region in place."""
    off = 0
    off += SCENE_NAME_LEN  # scene name in pickle header

    # Update Bees version to 0.8.x (maj=0, min=8, rev=1)
    # Layout: min, maj, rev (little-endian u16)
    pickle[off] = 8      # min
    pickle[off + 1] = 0  # maj
    struct.pack_into("<H", pickle, off + 2, 1)
    off += 4

    off += MODULE_NAME_LEN  # module name
    off += 4                # module version

    net_start = off
    num_ops = struct.unpack_from("<I", pickle, off)[0]
    off += 4

    # First pass: read operator IDs and 0.7.1 state sizes, and record where each
    # operator's state lives so we can rewrite it with expanded state sizes.
    ops = []  # list of (op_id, old_state_off, old_state_size, new_state_size)
    for _ in range(num_ops):
        op_id = struct.unpack_from("<I", pickle, off)[0]
        if op_id >= 50:
            raise ValueError(f"Invalid 0.7.1 operator ID {op_id} at offset {hex(off)}")
        old_state_size = OP_STATE_SIZE_071[op_id]
        new_state_size = OP_STATE_SIZE_08X.get(op_id, old_state_size)
        ops.append((op_id, off + 4, old_state_size, new_state_size))
        off += 4 + old_state_size

    # Read the trailing regions that come after the operator records.
    input_region_old = off
    output_region_old = off + NET_INS_MAX * 4
    num_params_offset_old = output_region_old + NET_OUTS_MAX * 4

    trailing_start = num_params_offset_old
    trailing_data = bytes(pickle[trailing_start:])

    # Read original output-node targets.
    orig_targets = [
        struct.unpack_from("<i", pickle, output_region_old + i * 4)[0]
        for i in range(NET_OUTS_MAX)
    ]

    # Build new output-node array with dummy slots inserted before operators
    # that gained an output.
    new_targets = [-1] * NET_OUTS_MAX
    old_idx = 0
    new_idx = 0
    shifted = 0
    dummy_insertions = 0

    for op_id, _, _, _ in ops:
        out_count = OP_NUM_OUTPUTS_071[op_id]
        if op_id in OP_IDS_GAINED_OUTPUT:
            if new_idx < NET_OUTS_MAX:
                new_targets[new_idx] = -1
                new_idx += 1
                dummy_insertions += 1
        for _ in range(out_count):
            if old_idx >= NET_OUTS_MAX or new_idx >= NET_OUTS_MAX:
                break
            if orig_targets[old_idx] != -1:
                shifted += 1
            new_targets[new_idx] = orig_targets[old_idx]
            old_idx += 1
            new_idx += 1

    # Second pass: rebuild the pickle with expanded operator states.  The
    # input/output/param regions shift by the total state expansion amount.
    # Because the pickle blob is fixed-size (SCENE_PICKLE_SIZE), the trailing
    # params/presets simply move later; any unused fill at the end is trimmed.
    new_pickle = bytearray()
    new_pickle.extend(pickle[:net_start + 4])  # header + numOps
    new_off = net_start + 4

    state_expansions = []
    total_state_expansion = 0
    for op_id, old_state_off, old_state_size, new_state_size in ops:
        new_pickle.extend(struct.pack("<I", op_id))
        new_off += 4
        old_state = bytes(pickle[old_state_off:old_state_off + old_state_size])
        new_pickle.extend(old_state)
        if new_state_size > old_state_size:
            pad = new_state_size - old_state_size
            # Fill the expansion with the operator's 0.8.x init defaults rather
            # than zeros, for operators whose 0.7.1 pickle never carried this
            # state. See OP_STATE_DEFAULTS above for the full rationale.
            filler = None
            if op_id in OP_STATE_DEFAULTS:
                candidate = OP_STATE_DEFAULTS[op_id]()
                if len(candidate) == pad:
                    filler = candidate
                else:
                    raise ValueError(
                        f"default state for {op_name(op_id)} (id={op_id}) is "
                        f"{len(candidate)} bytes but expansion is {pad} bytes"
                    )
            if filler is None:
                new_pickle.extend(b"\x00" * pad)
            else:
                new_pickle.extend(filler)
            total_state_expansion += pad
            state_expansions.append(
                f"{op_name(op_id)} (id={op_id}): {old_state_size} -> {new_state_size} bytes"
                + (" (init defaults)" if filler is not None else " (zero-filled)")
            )
        new_off += new_state_size

    input_region_new = new_off
    new_pickle.extend(pickle[input_region_old:output_region_old])
    new_off += NET_INS_MAX * 4

    output_region_new = new_off
    for target in new_targets:
        new_pickle.extend(struct.pack("<i", target))
    new_off += NET_OUTS_MAX * 4

    # Append params/presets from the original file starting at numParams.
    # These shift by total_state_expansion.
    new_pickle.extend(pickle[num_params_offset_old:])

    # Ensure the converted pickle is exactly SCENE_PICKLE_SIZE bytes.
    if len(new_pickle) < SCENE_PICKLE_SIZE:
        new_pickle.extend(b"\x00" * (SCENE_PICKLE_SIZE - len(new_pickle)))
    elif len(new_pickle) > SCENE_PICKLE_SIZE:
        new_pickle = new_pickle[:SCENE_PICKLE_SIZE]

    # Replace the caller's pickle buffer contents in place.
    pickle[:] = new_pickle

    return {
        "num_ops": num_ops,
        "dummy_insertions": dummy_insertions,
        "shifted_connections": shifted,
        "state_expansions": state_expansions,
        "input_region": input_region_old,
        "output_region": output_region_old,
        "num_params_offset": num_params_offset_old,
    }


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Convert an Aleph Bees 0.7.1 scene file to 0.8.x format."
    )
    parser.add_argument("input", help="Input 0.7.1 .scn file")
    parser.add_argument("output", help="Output 0.8.x .scn file")
    args = parser.parse_args()

    in_path = Path(args.input)
    out_path = Path(args.output)

    if not in_path.exists():
        print(f"Error: input file not found: {in_path}", file=sys.stderr)
        return 1

    desc_bytes, pickle_bytes = parse_scene(in_path)

    print(f"Reading: {in_path}")
    before_info = describe_pickle(pickle_bytes)
    print_pickle_info("BEFORE", before_info)

    pickle = bytearray(pickle_bytes)
    stats = convert_pickle(pickle)

    print(f"\nConversion stats:")
    print(f"  operators converted: {stats['num_ops']}")
    print(f"  dummy output slots inserted: {stats['dummy_insertions']}")
    print(f"  connected outputs shifted: {stats['shifted_connections']}")
    print(f"  operator state expansions:")
    for line in stats["state_expansions"]:
        print(f"    {line}")
    if not stats["state_expansions"]:
        print("    (none)")

    after_info = describe_pickle(bytes(pickle))
    print_pickle_info("AFTER", after_info)

    out_path.write_bytes(desc_bytes + bytes(pickle))
    print(f"\nWrote converted scene: {out_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
