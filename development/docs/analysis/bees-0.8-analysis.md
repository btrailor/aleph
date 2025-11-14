# BEES 0.8.x - Unofficial Release Analysis

## Overview

BEES 0.8.x was an **unofficial community release** developed by contributor **boqs** and others, released in January 2018 after years of development on the `dev` branch. It represents significant improvements over the last official release (0.7.1 from 2014).

**Release Date**: January 22, 2018  
**Primary Developer**: boqs (with contributions from catfact, tehn, and community)  
**Status**: Unofficial/community release, never officially tagged  
**Download**: sdcard-0.8.1.zip (777.1 KB) - distributed via lines forum  

---

## Breaking Changes

### Scene Incompatibility
**Critical**: 0.8.x is **incompatible with all previous scenes** (0.7.x and earlier)

**Reasons**:
1. New memory management system
2. Operator architecture changes
3. Preset system overhaul
4. Parameter storage format changes

**Migration Path**:
- Scenes from recent `dev` branch builds might work
- Older scenes need manual reconstruction
- Scene format fundamentally changed

---

## Major Features & Improvements

### 1. Fixed Preset System ✅
**Problem Solved**: Preset system in 0.7.x was buggy/unreliable

**Improvements**:
- Stable preset storage and recall
- Real-time preset manipulation works correctly
- Conditional inclusion/exclusion of components
- More than 32 presets per scene (expandable)
- OUTPUT routings now included in presets

**Impact**: High - This was a major pain point for users

### 2. Advanced Patch Editing ✅
**Enhancements**:
- Better network editing workflow
- Improved connection management
- More intuitive routing interface
- Visual feedback improvements

**Impact**: High - Core user experience improvement

### 3. Memory Management Overhaul ✅
**From PR #267** (Net use malloc):

**Old System**:
- Static operator pool (fixed number)
- Pre-allocated memory for all ops
- No dynamic creation/deletion
- Memory wasted on unused operators

**New System**:
- Dynamic allocation via malloc/free
- Arbitrary operator deletion/insertion
- Memory pools for small/large ops
- Operators created on demand

**Benefits**:
- More flexible patching
- Can delete operators mid-patch
- Better memory utilization
- More complex scenes possible

**Technical Details**:
```c
// Old way: Static pool
op_t operators[MAX_OPS];  // Wasteful

// New way: Dynamic allocation
op_t* op = malloc(sizeof(op_t));
// ... use operator ...
free(op);  // Clean up when done
```

**Code Changes**:
- 44 commits in PR #267
- Affected `apps/bees/src/net.c`
- New memory pool system
- Fall-back to regular malloc if pool exhausted

### 4. CV Output Improvements ✅
**Issue #294 addressed**:

**Problems Fixed**:
- CV glitches when multiple encoders moved simultaneously
- Random voltage spikes on outputs
- 50µs pulse bursts appearing on CV outs
- Synchronization issues between AVR32 and Blackfin

**Solutions**:
- Better SPI parameter throttling
- Fixed DAC communication timing
- Encoder polling improvements
- Prevents multiple param updates in single audio frame

**Impact**: Critical - Affects live performance reliability

### 5. Lines Module Enhancements ✅
**From Issue #304**:

**Fixed**: Record write head clicks/pops

**Commit f95ccd6**:
- Added smoothing to write operations
- Reduced audible artifacts when punching in/out
- Better handling of loop transitions

**Planned for post-0.8**:
- Fixed slew on pre & write controls
- Further artifact reduction

### 6. Other Improvements

**Filesystem Stability**:
- Better SD card handling
- More robust scene loading
- Improved error recovery

**Performance**:
- Optimized network routing
- Faster operator execution
- Reduced latency in certain operations

**Bug Fixes**:
- Numerous stability improvements
- Fixed memory leaks
- Better error handling throughout

---

## What 0.8.x Enables

### Complex Patching Workflows
With dynamic operators, users can now:
- Build and tear down networks on the fly
- Experiment without running out of operators
- Create more sophisticated routing
- Live-code patches in performance

### Stable Performance
Fixed CV glitches mean:
- Reliable live use
- Smooth parameter automation
- Multiple simultaneous modulations
- Professional-grade stability

### Better Creative Experience
Improved preset system enables:
- Scene morphing
- A/B comparison
- Performance presets
- Reliable state recall

---

## Technical Lessons for 1.0 Development

### 1. Memory Management is Critical
**Lesson**: Dynamic allocation was a game-changer

**For 1.0**:
- Must include PR #267 changes
- Thorough testing required (fragmentation risks)
- Memory profiling tools needed
- Stress testing essential

### 2. Scene Format Stability Matters
**Lesson**: Breaking compatibility has costs

**For 1.0**:
- Consider version migration tools
- Document format changes clearly
- Provide conversion utilities
- Maintain backward compatibility where possible

### 3. CV Output Timing is Delicate
**Lesson**: SPI timing issues are subtle

**For 1.0**:
- Keep cv.c improvements from 0.8
- Test with multiple simultaneous modulations
- Verify on hardware with oscilloscope
- Stress test encoder + CV scenarios

### 4. Community-Driven Development Works
**Lesson**: boqs and community pushed it forward

**For 1.0**:
- Engage community early
- Beta testing program essential
- Regular progress updates
- Collaborative development model

---

## Code Locations & Branches

### Where to Find 0.8.x Code

**GitHub Branches**:
- Main development: `monome/aleph` `dev` branch
- Memory management: PR #267 (closed but not merged to master)
- boqs's work: Various commits in `dev` branch

**Key Contributors' Forks**:
- `boqs/aleph` - Memory management, CV fixes
- `catfact/aleph` - Core BEES, DSP modules  
- `rick-monster/aleph` - Network refactoring

**Important Commits**:
- f95ccd6: Lines write head fix
- ecf1b32: CV DAC changes (later reverted)
- a3eb744: Root cause CV glitch fix (boqs)
- cae19e5c9: Input/output refactoring (rick-monster)

### Build Artifacts
- aleph-bees-0.8.1.hex (from sdcard-0.8.1.zip)
- Updated DSP modules (.ldr files)
- New parameter descriptors (.dsc files)
- Example scenes for 0.8.x

---

## User Feedback & Reception

### Positive Response
From lines forum:
- "0.8.x allows to do many more things than 0.7.x"
- Preset system finally reliable
- Much more stable in performance
- Worth the scene incompatibility

### Community Scenes
Users started sharing 0.8-specific patches:
- Aiecho.scn - 8-bit space echo (by altoaiello)
- Various granular patches
- Complex multi-operator scenes
- Performance-oriented setups

### Known Issues (Post-0.8)
1. Scene compatibility pain point
2. Some users reported build issues
3. Documentation lagged behind features
4. No official binaries (DIY compilation)

---

## Integration Plan for BEES 1.0

### Must Include from 0.8.x

#### High Priority:
1. **Memory Management System** (PR #267)
   - Dynamic operator allocation
   - Arbitrary op deletion/insertion
   - Memory pool implementation
   - **Action**: Merge, test extensively, profile

2. **Fixed Preset System**
   - Stable storage/recall
   - OUTPUT routing in presets
   - Expandable preset count
   - **Action**: Include in Phase 1

3. **CV Output Fixes** (Issue #294)
   - SPI timing improvements
   - Parameter throttling
   - Encoder polling fixes
   - **Action**: Critical for Phase 1

#### Medium Priority:
4. **Advanced Patch Editing**
   - Improved UI workflows
   - Better visual feedback
   - **Action**: Phase 2

5. **Lines Improvements**
   - Write head click fixes
   - Loop transition smoothing
   - **Action**: Phase 2-3

### Testing Requirements

**Before 1.0 Release**:
- [ ] Memory stress tests (24hr+ runtime)
- [ ] Fragmentation analysis
- [ ] CV output verification (scope)
- [ ] Multi-encoder modulation tests
- [ ] Scene load/save reliability (1000+ cycles)
- [ ] Preset recall accuracy tests
- [ ] Complex network performance tests

### Documentation Needs

**Critical Docs**:
1. **Migration Guide**: 0.7.x → 1.0
   - Scene conversion process
   - Breaking changes explained
   - Workarounds documented

2. **Memory Management Guide**:
   - How dynamic ops work
   - Best practices
   - Limitations

3. **Preset System Tutorial**:
   - New capabilities
   - Performance techniques
   - Scene design patterns

---

## Unanswered Questions

### Need to Investigate:
1. **Exact 0.8.1 build**:
   - Which specific commit?
   - Build date/version string?
   - Any patches after 0.8.1?

2. **Memory fragmentation**:
   - Real-world testing results?
   - Any reported issues?
   - Mitigation strategies used?

3. **Scene format details**:
   - Exact specification?
   - Documented anywhere?
   - Can we parse 0.8 scenes programmatically?

4. **Other unreleased features**:
   - What else is in `dev` branch?
   - Any half-finished features?
   - Technical debt created?

### How to Find Answers:
1. Contact boqs directly (GitHub user)
2. Search lines forum archives more thoroughly
3. Diff `dev` branch against last official release
4. Build and test 0.8.x ourselves
5. Reach out to active Aleph community members

---

## Recommendations for 1.0

### 1. Start from 0.8.x Foundation
**Rationale**: 
- 4+ years of development
- Battle-tested by community
- Major pain points addressed
- Proven stability improvements

**Action**:
- Use `dev` branch as base
- Don't start from 0.7.1
- Verify PR #267 is fully integrated
- Test all 0.8.x improvements

### 2. Official Release Process
**Learn from 0.8**:
- 0.8 stayed "unofficial" too long
- Users want stable releases
- Clear versioning matters

**For 1.0**:
- Proper semantic versioning
- Official GitHub releases
- Pre-built binaries
- Release notes with migration guides

### 3. Backward Compatibility Strategy
**Challenge**: 0.8 broke everything

**For 1.0**:
- Provide scene converter tool
- Support both old/new formats during transition
- Clear deprecation timeline
- Sample scenes in both formats

### 4. Community Engagement
**Success of 0.8**:
- Community drove it forward
- Multiple contributors
- Shared knowledge

**For 1.0**:
- Maintain this momentum
- Beta tester program
- Regular development updates
- Contributor recognition

---

## Critical Paths Forward

### Immediate Next Steps:

1. **Build & Test 0.8.1**
   - Get hardware
   - Build from `dev` branch
   - Test all reported improvements
   - Document current state

2. **Code Audit**
   - Review PR #267 thoroughly
   - Check for technical debt
   - Identify risky changes
   - Plan refactoring if needed

3. **Community Outreach**
   - Contact boqs about status
   - Ask lines community for feedback
   - Gather 0.8.x scene examples
   - Understand real-world usage

4. **Establish Baseline**
   - `dev` branch as 1.0 starting point
   - List all changes since 0.7.1
   - Prioritize what to keep/improve
   - Create clean commit history

---

## Conclusion

BEES 0.8.x represents **significant progress** over 0.7.1, with critical fixes and features that make Aleph much more usable. The unofficial nature and scene incompatibility held it back from wider adoption, but the technical improvements are solid.

**For BEES 1.0**: 
- Build on 0.8.x foundation
- Add missing features (FX system, Teletype port)
- Polish to professional quality
- Release officially with proper support

The community has done the hard work of advancing BEES significantly. The path to 1.0 should honor that work while taking it to the next level.

---

*Document Version: 1.0*  
*Date: November 2025*  
*Based on: community forums, GitHub issues, PR #267, dev branch analysis*
