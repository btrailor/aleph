# BEES 1.0 Development Quick Start Guide

## TL;DR - Start Here

```bash
# 1. Clone the RIGHT branch (dev, not master!)
git clone https://github.com/monome/aleph.git
cd aleph
git checkout dev  # This has 0.8.x code

# 2. You now have BEES 0.8.x - the actual current version
# DO NOT use master branch (that's old 0.7.1)

# 3. Set up toolchains (see detailed instructions below)
# 4. Build and test 0.8.x
# 5. Then start planning 1.0 features
```

## Critical First Understanding

### The Version Situation
- **master branch**: 0.7.1 (November 2014) - OLD, don't use
- **dev branch**: 0.8.x (January 2018) - CURRENT, start here ⭐
- **Your goal**: 1.0 (Future) - build on dev branch

### Why dev branch?
0.8.x solved major problems:
- ✅ Dynamic memory management
- ✅ CV output glitches  
- ✅ Preset system
- ✅ Stability improvements

Starting from master means re-solving these. Don't do it.

## Phase 0: Verify 0.8.x Baseline (Month 0)

### Week 1: Source & Environment

**Day 1-2: Get the code**
```bash
# Fork the repo on GitHub first, then:
git clone https://github.com/YOUR_USERNAME/aleph.git
cd aleph
git checkout dev
git remote add upstream https://github.com/monome/aleph.git

# Tag this as your baseline
git tag 0.8.x-baseline
```

**Day 3-7: Set up toolchains**

*AVR32 (for BEES - controller)*
```bash
# Mac:
# See: https://github.com/droparea/avr32-toolchain

# Linux:
# Download from Atmel (need account):
# - avr32-gnu-toolchain-3.4.2.435-linux.any.x86.tar.gz
# - atmel-headers-6.1.3.1475.zip
# Extract and add to PATH
```

*Blackfin (for DSP - audio)*
```bash
# Use Docker (easiest):
docker pull pf0camino/cross-bfin-elf

# Or install manually:
# Download from: http://blackfin.uclinux.org/
# Version: 2012R2-RC2 or newer
```

**Test the build**
```bash
# Build BEES
cd apps/bees
make

# Should produce: aleph-bees.hex
# If it fails, check toolchain setup

# Build a module
cd ../../modules/lines
make

# Should produce: lines.ldr and lines.dsc
```

### Week 2: Hardware Testing

**You'll need**:
- Aleph hardware unit
- SD card (FAT formatted)
- USB keyboard
- Audio cables
- Oscilloscope (for CV testing - optional but recommended)

**Test procedure**:
1. Copy built files to SD card
2. Boot Aleph in bootloader mode
3. Flash aleph-bees.hex
4. Test basic functionality
5. Test dynamic operators (create/delete)
6. Test CV outputs (check for glitches)
7. Test preset save/recall
8. Test scene save/load

**Create test scenes**:
- Simple: 10 operators, basic routing
- Medium: 30 operators, complex network
- Stress: 50+ operators, max connections

### Week 3: Code Review

**Priority reading order**:

1. **Memory Management** (PR #267)
   - `apps/bees/src/net.c` - Dynamic allocation
   - Look for `malloc()` and `free()` calls
   - Understand memory pools

2. **CV Output Fixes** (Issue #294)
   - `bfin_lib/src/cv.c` - DAC communication
   - Look for SPI timing changes
   - Parameter throttling code

3. **Preset System**
   - `apps/bees/src/preset.c`
   - Scene save/load code
   - Storage format

4. **Network Routing**
   - `apps/bees/src/net.c`
   - `apps/bees/src/net_protected.h`
   - Operator connections

### Week 4: Documentation

**Create these documents**:

1. **0.8.x Feature List**
   - What works
   - What's changed since 0.7.1
   - Known issues
   
2. **Architecture Map**
   - Key components
   - Data flow
   - Memory layout
   
3. **Build Instructions**
   - Step-by-step for your platform
   - Common problems and solutions
   
4. **Test Procedures**
   - How to verify functionality
   - Regression test checklist

**Deliverable**: 
- Development environment working
- 0.8.x builds and runs
- Code understood
- Ready to start 1.0 work

## What to Test First

### Critical Path Tests

**Memory Management**:
```
Test: Dynamic Operator Creation
1. Start with clean scene
2. Create 20 operators via UI
3. Delete 10 operators
4. Create 15 more operators
5. Run for 1 hour
6. Check for memory leaks
Expected: No crashes, stable memory usage
```

**CV Outputs**:
```
Test: Multi-Modulation Stability  
1. Connect 4 encoders to 4 CV outputs
2. Move all encoders simultaneously
3. Monitor outputs with oscilloscope
Expected: Clean CV, no glitches or spikes
```

**Presets**:
```
Test: Preset Reliability
1. Create complex scene (20+ ops)
2. Store 8 presets with different values
3. Recall each preset 10 times
4. Compare values
Expected: 100% accuracy, no corruption
```

**Stability**:
```
Test: Long-Duration  
1. Complex scene running
2. Continuous modulation
3. Run for 24 hours minimum
Expected: No crashes, no memory leaks
```

## Common Gotchas

### Build Issues

**Problem**: "avr32-gcc not found"
- **Solution**: Add toolchain to PATH
- Check: `which avr32-gcc` should show path

**Problem**: "Atmel headers missing"
- **Solution**: Extract headers to toolchain include dir
- Location: `toolchain/avr32/include/`

**Problem**: "bfin-elf-gcc not found"  
- **Solution**: Use Docker or install toolchain
- Check: `which bfin-elf-gcc`

### Code Issues

**Problem**: Scenes from 0.7.x don't load
- **Expected**: This is normal, 0.8.x broke compatibility
- **Solution**: Will need migration tools (Phase 1 work)

**Problem**: Weird memory behavior
- **Check**: Using dev branch, not master?
- **Check**: PR #267 code present?

## Quick Reference: Key Locations

### Important Files
```
apps/bees/               # BEES application
  src/
    net.c               # Network routing + malloc/free
    handler.c           # Event handling
    preset.c            # Preset system
    pages/              # UI pages
    ops/                # Operators
  
bfin_lib/src/
  cv.c                  # CV output (with fixes)
  init.c                # Blackfin initialization
  
modules/
  lines/                # Loop module
  waves/                # Oscillator module
  (others)              # More modules
```

### Important Branches
- `dev` - 0.8.x code (USE THIS)
- `master` - 0.7.1 code (DON'T USE)
- Forks to check:
  - `boqs/aleph` - Original 0.8.x work
  - `rick-monster/aleph` - Network experiments

### GitHub Issues
- #267 - Memory management (MERGED to dev)
- #294 - CV glitches (FIXED in 0.8.x)
- #87 - Input/output refactor (IN PROGRESS)
- #79 - SD card issues (PARTIAL in 0.8.x)

## Next Steps After Phase 0

Once you've verified 0.8.x baseline:

1. **Issue Triage**
   - Separate solved vs. open issues
   - Prioritize remaining work
   - Create 1.0 milestone

2. **Community Outreach**
   - Contact boqs about 0.8.x
   - Ask lines community for feedback
   - Announce 1.0 development plan

3. **Planning**
   - What needs to be added?
   - What needs to be fixed?
   - What's the timeline?

4. **Start Phase 1**
   - Begin stabilization work
   - Fix remaining critical bugs
   - Prepare for new features

## Resources

### Documentation
- Aleph docs: http://monome.org/docs/aleph
- lines forum: https://llllllll.co
- GitHub: https://github.com/monome/aleph

### Community
- Forum: lines (llllllll.co)
- Support: help@monome.org
- Contributors: boqs, catfact, tehn, rick-monster

### Tools
- AVR32 toolchain: https://github.com/droparea/avr32-toolchain
- Blackfin Docker: pf0camino/cross-bfin-elf
- Development guides: In repo under `apps/bees/docs/`

## Getting Help

**If stuck**:
1. Check existing GitHub issues
2. Search lines forum
3. Review 0.8.x analysis document
4. Ask on lines forum
5. Email help@monome.org

**Before asking**:
- ✅ Verified you're on dev branch
- ✅ Tried clean rebuild
- ✅ Checked documentation
- ✅ Searched for similar issues

## Success Checklist

Phase 0 complete when you can check all:

- [ ] Built BEES 0.8.x from dev branch
- [ ] Built at least 2 DSP modules
- [ ] Tested on hardware successfully
- [ ] Dynamic operators work (create/delete)
- [ ] CV outputs stable (no glitches)
- [ ] Presets save/recall correctly
- [ ] Understand memory management (PR #267)
- [ ] Understand CV fixes (Issue #294)
- [ ] Documented current state
- [ ] Ready to start 1.0 development

## Timeline

- Week 1: Source control & build environment
- Week 2: Hardware testing & validation
- Week 3: Code review & understanding
- Week 4: Documentation & planning

**Total: 1 month to Phase 0 completion**

Then move to Phase 1 (Stabilization) with confidence!

---

*Quick Start Guide v1.0*  
*For BEES 1.0 Development*  
*Foundation: BEES 0.8.x (dev branch)*  
*November 2025*
