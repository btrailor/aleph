# CDC Grid Support for Aleph - Work Summary

## Project Goal

Add support for modern monome grid devices (USB-C, VID 0x0483) which use CDC (Communications Device Class) USB transport instead of the legacy FTDI transport used by older grids.

## Current Status: BLOCKED

Grid enumerates successfully via CDC driver, but remains non-functional due to transport layer confusion between CDC and FTDI.

---

## Work Completed

### 1. CDC Driver Implementation (✅ Complete)

- **Location**: `libavr32/src/usb/cdc/uhi_cdc.c`, `cdc.c`
- **Changes**:
  - Added VID 0x0483 support (modern grids) alongside legacy 0x16c0
  - Fixed interface tracking: Set `b_iface_comm_supported` and `b_iface_data_supported` flags
  - Verified CDC driver successfully enumerates device with all endpoints

### 2. USB Driver Order Fix (✅ Complete)

- **Location**: `libavr32/conf/aleph/conf_usb_host.h`
- **Issue**: HID driver was claiming grid before CDC could enumerate
- **Fix**: Reordered to FTDI, CDC, HID, MIDI so CDC gets priority over HID

### 3. String Encoding Fix (✅ Complete)

- **Location**: `libavr32/src/monome.c` line 277
- **Issue**: Code assumed all strings were Unicode (UTF-16) like FTDI, but CDC uses ASCII
- **Fix**: Added `isUnicode` flag based on transport type:
  ```c
  u8 isUnicode = (detectedTransport == eTransportFTDI);
  ```

### 4. Serial Pattern Matching Fix (✅ Complete)

- **Location**: `libavr32/src/monome.c` lines 310-350
- **Issue**: Code only recognized serials starting with 'm' (like "m128-xxx")
- **Modern grids use**: "cdc001", "cdc002", etc.
- **Fix**: Accept non-'m' serials and assume modern extended protocol (eProtocolMext)

### 5. Grid Size Simplification (✅ Complete)

- **Issue**: Attempting `setup_mext()` device query caused infinite USB RX error loop (status 0x00000007)
- **Fix**: Skip query, hardcode 128 grid size (16x8) for non-'m' serial patterns
- **Rationale**: Most common modern grid size, avoids problematic device communication

### 6. Transport Detection Debugging (✅ Complete)

- **Location**: `libavr32/src/monome.c` lines 599-615
- **Added**: Debug output showing which transport is detected (CDC vs FTDI)

### 7. FTDI Callback Guard (✅ Complete - But Insufficient)

- **Location**: `libavr32/src/usb/ftdi/ftdi.c` line 52
- **Added**: Check `if (ftdiConnect)` before processing FTDI RX data
- **Result**: Did not solve the problem

---

## The Unsolved Problem

### Symptoms

```
USB: device connection, present=1 dev=0x00001E78
FTDI: uhi_ftdi_install() called
FTDI: VID/PID not matched, rejecting
CDC: uhi_cdc_install() called
CDC: VID accepted (00000483), proceeding with enumeration
CDC: found communication interface
CDC: found data interface
CDC: device install SUCCESS
CDC device enabled
USB: enumeration complete, status=00000000
CDC setup routine
CDC strings: man=monome prod=grid ser=cdc001
detect_transport: CDC connected          <-- CORRECT
check_monome: processed manstring: monome matchMan=00000001
detected modern grid, assuming 128 (16x8)
CDC device check result: 00000001        <-- SUCCESS

ftdi rx transfer error received          <-- PROBLEM STARTS HERE
ftdi rx transfer error
ftdi rx transfer error
...
```

### Root Cause Analysis

**The issue occurs at multiple levels:**

1. **Event Handler Problem** (Primary Issue)

   - **Location**: `avr32/src/main.c` lines 95-110
   - Aleph only has `handler_FtdiConnect()` which calls `ftdi_setup()`
   - **monome's teletype has TWO handlers:**

     ```c
     static void handler_FtdiConnect(s32 data) {
         ftdi_setup();
     }

     static void handler_SerialConnect(s32 data) {  // FOR CDC!
         monome_setup_mext();
     }
     ```

   - Aleph treats both FTDI and CDC as "FTDI connect" events

2. **FTDI RX Callbacks Still Active**

   - Even though FTDI driver rejected the device (`UHC_ENUM_UNSUPPORTED`)
   - Even though `uhi_ftdi_enable()` returns early
   - FTDI RX transfer callbacks are still being triggered by USB infrastructure
   - When data arrives, `ftdi_rx_done()` callback fires and calls `monome_read_serial()`

3. **Transport Layer Confusion**
   - `detect_transport()` correctly identifies CDC initially
   - But somewhere in the code flow, it's called again or transport functions are invoked incorrectly
   - FTDI transport functions are being called on CDC device data

### Why Our Fixes Didn't Work

1. **FTDI Callback Guard**: Added `if (ftdiConnect)` check

   - **Problem**: `ftdiConnect` flag may be set incorrectly or checked at wrong time
   - **Problem**: Callback is still registered and firing even for CDC devices

2. **Transport Detection**: Works correctly initially
   - **Problem**: Code architecture assumes only one transport type exists
   - **Problem**: No mechanism to prevent FTDI callbacks when CDC is active

---

## What Monome Does Differently

Based on analysis of `monome/teletype` repository:

### 1. Separate Event Handlers

```c
// teletype/module/main.c lines 547-578
static void handler_FtdiConnect(s32 data) {
    ftdi_setup();
}

static void handler_SerialConnect(s32 data) {  // CDC devices
    monome_setup_mext();
}

static void handler_FtdiDisconnect(s32 data) {
    grid_connected = 0;
    timers_unset_monome();
}
```

### 2. Event Type Differentiation

- `kEventFtdiConnect` - for FTDI devices
- `kEventSerialConnect` - for CDC devices
- `kEventSerialDisconnect` - for CDC disconnect

### 3. No Transport Auto-Detection

- Teletype doesn't try to detect transport after enumeration
- Each driver posts its own specific event type
- Event handlers know exactly which transport to set up

---

## Architectural Issues in Aleph

### The Dual-Path Problem

**Path 1: USB Enumeration** (Working)

```
1. USB device connects
2. UHC tries each driver in order: FTDI, CDC, HID, MIDI
3. FTDI rejects (VID mismatch)
4. CDC accepts and enumerates successfully
5. CDC calls check_monome_device_desc()
6. Device recognized, transport set to CDC
```

**Path 2: Data Transfer** (Broken)

```
1. USB data arrives on endpoints
2. USB controller triggers endpoint callbacks
3. FTDI RX callback fires (WHY??)
4. FTDI callback calls monome_read_serial()
5. Protocol layer tries to read from wrong transport
6. Errors occur
```

### Core Problem

**Aleph's USB architecture allows multiple drivers' callbacks to remain active even when a device is claimed by a different driver.**

The FTDI RX callback is registered during `uhi_ftdi_install()` and remains active even after the function returns `UHC_ENUM_UNSUPPORTED`. When USB data arrives, the USB controller doesn't know which driver should handle it - it just fires all registered callbacks.

---

## Attempted Solutions & Why They Failed

### Attempt 1: Guard FTDI callback with connection flag

```c
if (ftdiConnect) {
    (*monome_read_serial)();
}
```

**Result**: Still getting FTDI errors
**Why**: `ftdiConnect` flag management may be incorrect, or callback is triggered before flag is checked

### Attempt 2: Transport detection and switching

**Result**: Correctly detects CDC initially, but then FTDI errors occur
**Why**: Detection happens at setup time, but callbacks fire at runtime independent of detection

### Attempt 3: Skip problematic device query

**Result**: Avoids infinite loop, but doesn't fix transport confusion
**Why**: Addresses symptom (query causing errors) not root cause (wrong transport being used)

---

## Potential Solutions (Not Implemented)

### Solution 1: Disable FTDI Callbacks for CDC Devices

**Approach**: When CDC device is detected, explicitly unregister or disable FTDI RX callbacks
**Challenges**:

- May require modifying ASF USB host controller code
- Need to understand callback registration mechanism
- Risk of breaking FTDI support

### Solution 2: Add kEventSerialConnect Handler (Teletype's Approach)

**Approach**:

1. Add new event type `kEventSerialConnect`
2. Have CDC driver post this event instead of `kEventFtdiConnect`
3. Create separate handler that sets up CDC transport without touching FTDI
4. Ensure FTDI and CDC paths are completely independent

**Challenges**:

- Requires refactoring event handling system
- Need to audit all places that assume "monome connect" == FTDI
- May break existing functionality

### Solution 3: Transport-Aware Callback Dispatching

**Approach**: Add transport type check in callback dispatcher
**Challenges**:

- Need to track which transport is active globally
- Callbacks need access to transport state
- May require significant refactoring

### Solution 4: Reference Teletype's libavr32

**Approach**:

- Compare Aleph's libavr32 fork with teletype's libavr32
- Identify key differences in USB/monome handling
- Port relevant changes

**Challenges**:

- Repos may have diverged significantly
- Risk of breaking existing Aleph functionality
- May require extensive testing

---

## Technical Details

### Key Files

- **libavr32/src/monome.c**: Core monome device handling, protocol, transport
- **libavr32/src/usb/cdc/cdc.c**: CDC transport layer
- **libavr32/src/usb/cdc/uhi_cdc.c**: CDC USB host interface driver
- **libavr32/src/usb/ftdi/ftdi.c**: FTDI transport layer
- **libavr32/src/usb/ftdi/uhi_ftdi.c**: FTDI USB host interface driver
- **libavr32/conf/aleph/conf_usb_host.h**: USB driver configuration
- **avr32/src/main.c**: Main event loop and handlers

### Git Commits

- Branch: `cdc-dev` (v0.8.2)
- Latest commit: da543f45 "Update libavr32: fix FTDI RX callback for CDC devices"
- libavr32 submodule: 7967d1d "Fix FTDI RX callback firing for CDC devices"

### Debug Output Format

Serial console at `/dev/tty.usbmodem12501` @ 115200 baud shows:

- USB enumeration sequence
- Driver install/reject messages
- Transport detection
- Device string parsing
- Error messages (when they occur)

---

## Recommendations

### Short Term

1. **Do NOT merge to main** - code is non-functional despite successful enumeration
2. Document this work for future reference
3. Consider if CDC support is worth the architectural changes required

### Long Term

1. **Study teletype's approach**:

   - Their event handling architecture
   - How they separate FTDI and CDC paths
   - Their libavr32 implementation differences

2. **Consider architectural refactoring**:

   - Separate FTDI and CDC event paths completely
   - Implement proper callback management
   - Add transport-aware dispatching

3. **Alternative: Wait for modern FTDI-based grids**:
   - If monome releases FTDI-based USB-C grids, problem is avoided
   - Legacy FTDI support already works in Aleph

---

## Questions for Future Investigation

1. **Why do FTDI callbacks fire for CDC devices?**

   - Is this ASF USB stack behavior?
   - Can it be prevented at USB controller level?

2. **What does teletype's libavr32 do differently?**

   - Are there callback management functions we're missing?
   - Do they handle USB endpoints differently?

3. **Can we disable FTDI driver completely for CDC devices?**

   - What's the proper way to unregister callbacks?
   - Would this break hot-swapping between device types?

4. **Is there a simpler fix we're missing?**
   - Some flag or configuration we haven't found?
   - A callback priority system?

---

## Conclusion

Modern monome grids successfully enumerate via CDC and are correctly identified as "monome grid" devices. All string parsing, transport detection, and device recognition work correctly. However, the fundamental architectural issue of FTDI callbacks remaining active for CDC devices prevents actual grid functionality.

The fix requires either:

1. Deep refactoring of the USB/monome event handling architecture (following teletype's model)
2. Low-level USB callback management changes to prevent cross-transport interference
3. Finding a simpler solution by studying the exact differences between Aleph's and teletype's libavr32 implementations

**This is a solvable problem, but requires more architectural understanding than we currently have.**

---

## Files Modified

### libavr32 submodule

- `src/usb/cdc/uhi_cdc.c` - VID support, interface tracking
- `src/usb/cdc/cdc.c` - Setup and string handling
- `src/usb/ftdi/ftdi.c` - Callback guard (insufficient)
- `src/monome.c` - String encoding, serial patterns, transport detection, grid size
- `conf/aleph/conf_usb_host.h` - Driver order, CDC driver registration

### Main repository

- No changes to main Aleph code (only libavr32 submodule updated)

---

_Document created: November 13, 2025_
_Branch: cdc-dev_  
_Status: Work suspended - requires architectural changes_
