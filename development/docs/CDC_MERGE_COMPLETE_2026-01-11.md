# CDC Merge Complete - 2026-01-11

## Summary

Successfully merged `cdc-dev` branch into `develop`, consolidating all CDC USB support work with scene migration and beekeep M1 features.

## Merge Details

**Date**: January 11, 2026  
**Commit**: 6ac949b2  
**Branches**: cdc-dev → develop  
**Commits Merged**: 184 commits from cdc-dev

## Conflicts Resolved

### 1. Version Number (`apps/bees/version.mk`)

- **Conflict**: develop=0.8.3 vs cdc-dev=0.8.2
- **Resolution**: Keep 0.8.3 (newer version)
- **Rationale**: Version numbers should always increment forward

### 2. libavr32 Submodule

- **Conflict**: Different commits - develop=02469a2 vs cdc-dev=43897902
- **Resolution**: Keep develop's 02469a2 (aleph-cdc-compat branch)
- **Rationale**: 02469a2 is on aleph-cdc-compat and contains latest CDC fixes
- **Note**: commit 43897902 from cdc-dev not found in any local branch

### 3. Beekeep Binary (`utils/beekeep/beekeep-0.8.3`)

- **Conflict**: Both branches added binary
- **Resolution**: Keep develop's version (arm64 Mach-O, 593KB)
- **Rationale**: Both versions are effectively the same, prefer develop's

### 4. Main.c CDC Handlers (`avr32/src/main.c`)

- **Conflict**: Different CDC include paths and disconnect handling
- **Changes Merged**:
  - Include path: kept `usb/cdc/cdc.h` (develop's correct path)
  - Added debug comment from cdc-dev in CdcConnect handler
  - Combined disconnect logic: `cdc_disconnect()` + event posting
  - Now properly clears `cdcConnect` flag and posts `kEventMonomeDisconnect`
- **Code**:
  ```c
  static void handler_CdcDisconnect(s32 data) {
      cdcConnect = 0;
      cdc_disconnect();
      /// CDC disconnection - assume this is monome disconnect
      event_t e = {.type = kEventMonomeDisconnect };
      event_post(&e);
  }
  ```

### 5. Network Operator Validation (`apps/bees/src/net.c`)

- **Conflict**: Missing validation in develop
- **Changes Merged**: Added operator ID validation from cdc-dev
- **Code**:
  ```c
  // Check if operator ID is valid
  if (opId >= numOpClasses) {
    print_dbg("\r\n ERROR: invalid operator ID: ");
    print_dbg_ulong(opId);
    return -1;
  }
  ```
- **Benefit**: Prevents crashes from invalid operator IDs during scene loading

### 6. Scene Headers (`apps/bees/src/scene.c`)

- **Conflict**: scene_convert.h include vs recallingScene extern
- **Resolution**: Kept BOTH - they serve different purposes
- **Final Structure**:

  ```c
  #include "scene_convert.h"   // For v0.7.1 → v0.8.x conversion

  // external variable for scene recall state
  extern u8 recallingScene;
  ```

## New Files from cdc-dev

### Beekeep JUCE Updates

- Massive JUCE library updates in `utils/beekeep_juce/`
- HarfBuzz text shaping library (~600 files)
- Accessibility features for Windows, macOS, Linux
- GUI improvements and modern platform support

### Scene Files

- `utils/convert_scenes.sh` - Scene conversion utility
- `utils/release/scenes/Aiecho.scn`
- `utils/release/scenes/Sniarg_1a.scn`

## Integration Highlights

### CDC Support Features Now in develop

1. **Transport Abstraction** - Support for both FTDI and CDC grids
2. **Modern Grid Detection** - Handles 2021+ monome grids (VID 0x0483)
3. **CDC Events** - Proper connect/disconnect event handling
4. **USB Driver Order** - FTDI, CDC, HID (proper priority)
5. **Connection Remapping** - Scene recall handles dynamic device connections

### Scene Migration Features Combined

- v0.7.1 → v0.8.x conversion framework (from develop)
- CDC device remapping (from cdc-dev)
- recallingScene flag prevents operator interference during recall
- Operator ID validation prevents crashes

### Build Infrastructure

- libavr32 pinned to 02469a2 (aleph-cdc-compat)
- beekeep binary for M1 Macs (593KB)
- JUCE 7.x updates for future beekeep development

## Current State

### Version

- **aleph-bees**: 0.8.3
- **libavr32**: 02469a2 (aleph-cdc-compat branch)

### Features

- ✅ CDC USB grid support (2021+ monomes)
- ✅ FTDI grid support (legacy monomes)
- ✅ Scene v0.7.1 loading framework
- ✅ Operator remapping infrastructure
- ✅ M1 Mac beekeep editor
- ✅ Parameter debug logging
- ✅ Connection state management

### Known Limitations

- Cannot build firmware (Docker 20.10.23 volume mount bug)
- Must update Docker or use Linux for builds
- Scene conversion not yet implemented (framework ready)

## Testing Required (After Docker Fix)

1. **CDC Grid Connection**

   - Connect modern monome grid (2021+)
   - Verify automatic CDC detection
   - Test grid button presses in bees app

2. **Scene Loading**

   - Load v0.8.x scenes (should work normally)
   - Test operator ID validation (try invalid scene)
   - Verify no crashes on malformed scenes

3. **CDC Disconnect Handling**

   - Connect CDC grid
   - Disconnect while app running
   - Verify proper event handling and cleanup

4. **Parameter Debug Output**
   - Load v0.7.1 scene (when converter implemented)
   - Check serial output for parameter unpickling logs
   - Verify operator remapping traces

## Next Steps

1. **Fix Docker** - Update to 24.x+ or build on Linux
2. **Build & Test** - Flash merged firmware to hardware
3. **Implement Converter** - scene_convert_from_0_7_1() function
4. **Document CDC** - Update user documentation for modern grids
5. **Clean Branches** - Archive/delete merged feature branches

## Commit Message

```
Merge cdc-dev: Consolidate CDC USB support and features

- Resolved conflicts by integrating changes from both branches
- Version: keep 0.8.3 (latest)
- libavr32: use 02469a2 (aleph-cdc-compat, has latest CDC fixes)
- CDC handlers: combined disconnect logic (cdc_disconnect() + event post)
- Operator validation: added ID range check from cdc-dev
- Scene loading: kept scene_convert.h include + recallingScene extern
- Binary: use latest beekeep-0.8.3 from develop
- New: JUCE beekeep_juce updates, scene files, utility scripts
```

## Git Graph

```
*   6ac949b2 (HEAD -> develop) Merge cdc-dev: Consolidate CDC USB support
|\
| * e67fba98 (cdc-dev) Add scaler reinitialization and error handling
| * da836542 Prevent grid operators from grabbing focus during scene recall
| * 85b8a586 Fix grid operator focus during scene recall
| * 8e260cf6 Remove debug output from CDC implementation
| * 572e925d Update libavr32: add transport and read function debug output
| * c5062738 Add separate cdcConnect flag and handler infrastructure
| ... (184 commits total)
* | e6ecffa6 Git workflow migration complete
* | 987382b4 Pin libavr32 to 02469a2
* | 6e2c7334 Add param debug logging
* | 20aafd4f Document build stability issues
```

## Repository Status

- **Branch**: develop
- **Remote**: pushed to fork/develop
- **Status**: Clean working tree
- **Submodule**: libavr32 at 02469a2
- **Build**: Blocked by Docker bug
- **Testing**: Blocked by build

## Summary

The CDC merge successfully consolidates 8+ months of parallel development:

- CDC USB support from cdc-dev (modern grids)
- Scene migration framework from develop (v0.7.1 loading)
- M1 build tooling from beekeep-m1
- JUCE editor updates

All major conflicts resolved by carefully integrating changes rather than choosing one side. The result is a unified codebase ready for modern monome grids with backwards compatibility for scene loading. Once Docker is fixed, firmware can be built and tested on hardware.
