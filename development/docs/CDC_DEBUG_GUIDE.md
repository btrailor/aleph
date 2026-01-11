# CDC Debug Output Guide

This guide explains the debug messages added to track CDC device connection for modern monome grids.

## Serial Connection Setup

**Settings:**

- Baud Rate: 57600
- Data: 8 bits, No parity, 1 stop bit
- Device: `/dev/cu.usbmodem*` (macOS)

**Connect:**

```bash
# Find device
ls /dev/cu.*

# Connect with screen
screen /dev/cu.usbmodem[TAB] 57600

# Exit: Ctrl-A then K, confirm with Y
```

## Expected Debug Messages

### When Grid is Plugged In

#### 1. USB Enumeration Start

```
run uhi_cdc_install
```

#### 2. VID/PID Detection

```
CDC install: VID=0x[value] PID=0x[value]
```

**What to check:**

- For modern grids (2021+): VID should be `0x0483` (STM32)
- For legacy grids: VID should be `0x16c0` (original monome)
- If VID is different, you'll see: `CDC: unsupported VID, rejecting`

#### 3. VID Accepted

```
CDC: VID accepted, parsing descriptors...
CDC: config descriptor length=[number]
```

#### 4. Interface Discovery

```
found CDC communication interface
allocating interrupt endpoint
found CDC data interface
allocating bulk endpoint
allocating bulk endpoint
```

#### 5. Interface Verification

```
CDC: interface check - comm:[1] data:[1] ep_in:0x[addr] ep_out:0x[addr]
```

**Success requires:**

- comm: 1 (communication interface found)
- data: 1 (data interface found)
- ep_in: non-zero (bulk IN endpoint allocated)
- ep_out: non-zero (bulk OUT endpoint allocated)

#### 6. Installation Result

**Success:**

```
CDC: SUCCESS - device installed!
```

**Failure:**

```
CDC: FAILED - interface not supported
```

#### 7. Device Enable

```
CDC: uhi_cdc_enable called
CDC: device enabled, calling cdc_change(true)
```

#### 8. Event Posting

```
cdc_change called, plug=1
cdc_change: posting kEventCdcConnect
```

#### 9. Setup Routine

```
CDC: setup routine starting
CDC: getting device strings...
CDC: Manufacturer='[string]' Product='[string]' Serial='[string]'
CDC: checking if monome device...
CDC: setup complete
```

#### 10. Main Application

```
got CDC monome device connection, saving flag for app launch
```

### When Grid is Unplugged

```
CDC: uhi_cdc_uninstall called
CDC: device uninstalled, calling cdc_change(false)
cdc_change called, plug=0
cdc_change: posting kEventCdcDisconnect
```

## Troubleshooting Based on Output

### Issue: No output at all

**Problem:** Serial connection not working
**Check:**

- Correct device selected
- Baud rate is 57600
- Debug firmware flashed (aleph-bees-0.7.1-dbg.hex)

### Issue: Stops at "CDC install: VID=..."

**Problem:** VID not recognized
**Check:**

- What VID is shown? Should be 0x16c0 or 0x0483
- If different, the grid may not be a monome device
- If it's 0x0483 but still rejected, VID fix didn't compile properly

### Issue: "CDC: unsupported VID, rejecting"

**Problem:** VID fix not applied or wrong VID
**Solution:**

- Verify firmware was built with VID fix
- Check what VID is being reported
- Modern grids should report 0x0483

### Issue: Gets past VID but no interfaces found

**Problem:** USB descriptor parsing issue
**Check:**

- Is "found CDC communication interface" printed?
- Is "found CDC data interface" printed?
- May indicate hardware or USB cable issue

### Issue: Interfaces found but "FAILED - interface not supported"

**Problem:** Endpoint allocation failed
**Check:**

- Look at "CDC: interface check" line
- comm and data should both be 1
- ep_in and ep_out should both be non-zero hex addresses
- If ep_in/ep_out are 0, USB host controller ran out of endpoints

### Issue: "SUCCESS" but no monome functionality

**Problem:** Higher-level monome protocol issue
**Check:**

- Does "got CDC monome device connection" appear?
- Does setup routine complete?
- Check manufacturer/product strings - should indicate monome
- May need to check monome protocol layer

## Common Success Pattern

Complete successful connection looks like:

```
run uhi_cdc_install
CDC install: VID=0x0483 PID=0x[pid]
CDC: VID accepted, parsing descriptors...
CDC: config descriptor length=[len]
found CDC communication interface
allocating interrupt endpoint
found CDC data interface
allocating bulk endpoint
allocating bulk endpoint
CDC: interface check - comm:1 data:1 ep_in:0x82 ep_out:0x02
CDC: SUCCESS - device installed!
CDC: uhi_cdc_enable called
CDC: device enabled, calling cdc_change(true)
cdc_change called, plug=1
cdc_change: posting kEventCdcConnect
CDC: setup routine starting
CDC: getting device strings...
CDC: Manufacturer='monome' Product='grid' Serial='[serial]'
CDC: checking if monome device...
CDC: setup complete
got CDC monome device connection, saving flag for app launch
```

## VID Reference

| VID    | Description             |
| ------ | ----------------------- |
| 0x16c0 | Original monome VID     |
| 0x0483 | STM32 VID (2021+ grids) |
| Other  | Not a recognized monome |

## Recording Debug Session

To save debug output to a file:

```bash
# Using script command
script aleph-debug-$(date +%Y%m%d-%H%M%S).log
screen /dev/cu.usbmodem* 57600
# Do your testing
# Exit screen: Ctrl-A, K
# Exit script: Ctrl-D
```

Or with direct capture:

```bash
cat /dev/cu.usbmodem* | tee aleph-debug-$(date +%Y%m%d-%H%M%S).log
```
