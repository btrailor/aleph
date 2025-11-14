# Beekeep Desktop Editor - Development Plan

## Overview

**Beekeep** is a graphical scene editor for Aleph BEES, allowing users to create and edit patches on their desktop computer rather than exclusively on the Aleph hardware. It provides a visual interface for operator networks, connections, and scene management.

## Current State

### What is Beekeep?

**Purpose**: Desktop patch editor for BEES scenes
**Interface**: Graphical node-based patching
**Format**: Reads/writes `.scn` (scene) files
**Features**:

- Create operators visually
- Connect inputs/outputs with mouse
- Edit parameters
- Save/load scenes
- Export to Aleph-compatible format

### Two Implementations

#### 1. Beekeep (GTK Version)

**Location**: `utils/beekeep/`  
**Technology**: GTK+ 3 (C/C++)  
**Status**: Last updated ~2014, basic functionality
**Platforms**:

- Linux: Works (32/64-bit binaries available)
- macOS: Problematic, especially on newer versions
- Windows: Untested/unsupported

**Known Issues**:

- Mac app bundle issues with GTK dependencies
- Missing icons/pixbufs on macOS
- Segfaults on quit (missing destroy handler)
- File selection dialog problems
- Designed as command-line tool, GUI is kludgy

**Last Release**: beekeep-0.6.0 (2014)

#### 2. Beekeep_JUCE (JUCE Version)

**Location**: `utils/beekeep_juce/`  
**Technology**: JUCE framework (C++)  
**Status**: Experimental, incomplete
**Purpose**: Cross-platform rewrite using JUCE

**Features**:

- Compiles BEES operators in C++
- OSC integration for communication
- Timers, MIDI support via JUCE
- Embryonic GUI (not finished)

**Issues**:

- Build problems against recent dev branch
- X11/JUCE message queue segfaults on Linux
- Missing source directories
- Abandoned around 2016

### Why Beekeep Matters

**Pain Points Without It**:

- Patching on Aleph hardware is tedious
- Small screen, 4 encoders + switches only
- Complex patches hard to visualize
- No undo/redo
- Slow iteration cycles

**Benefits With Working Beekeep**:

- Fast patch creation on desktop
- Visual network representation
- Copy/paste operators
- Experiment without hardware
- Scene library management
- Testing during development

## New Development Priorities (November 2025)

### Priority 1: Modern Monome Grid Support (HIGH PRIORITY)

**Current Limitation**:

- Aleph currently only supports **FTDI-based grids** (older models)
- Modern **CDC-based grids** (newer models) are not supported
- Eurorack modules already have updated libavr32 with CDC support

**Technical Solution**:

- **Source**: Updated libavr32 from eurorack modules codebase
- **Implementation**: Recompile firmware with updated USB driver support
- **Difficulty**: Moderate (mostly integration work)
- **Impact**: Enables compatibility with all modern monome grids

**Why High Priority**:

- **Hardware Compatibility**: Essential for users with newer grids
- **Community Access**: Most new users have CDC-based grids
- **Proven Solution**: libavr32 updates already tested in eurorack modules
- **Moderate Effort**: Leverages existing working code

**Technical Plan**:

1. **Week 1**: Extract updated libavr32 from eurorack modules
2. **Week 2**: Integrate CDC support into Aleph firmware
3. **Week 3**: Test with both FTDI and CDC grids
4. **Week 4**: Documentation and release

**Success Criteria**:

- [ ] Both FTDI and CDC grids work with Aleph
- [ ] No regression in FTDI grid support
- [ ] Updated firmware with grid auto-detection
- [ ] Tested with multiple grid models

### Priority 2: Beekeep Scene Version Migration System

**Current Problem**:

- Scene files from BEES 0.7 are incompatible with 0.8/1.0
- No automatic migration tool exists
- Users lose their existing patch libraries
- Testing new BEES versions is difficult

**Proposed Solution**: **Smart Scene Uploader**

- **Auto-detection**: Reads scene files and identifies BEES version
- **Migration Engine**: Converts scenes between versions automatically
- **Version Selector**: UI to choose target BEES version
- **Validation**: Ensures migrated scenes work correctly

**Why Important for Development**:

- **Testing**: Can use existing 0.7 scenes to test new firmware
- **User Adoption**: Preserves existing work when upgrading
- **Development Speed**: Faster testing with real-world scenes
- **Community Value**: Enables scene sharing across versions

**Technical Architecture**:

```
Scene File Input → Version Detector → Migration Engine → Target Version Output
                                   ↓
                             Version Selector UI
                                   ↓
                              Validation System
```

**Implementation Plan**:

1. **Week 1-2**: Scene format analysis (0.7 vs 0.8 vs 1.0)
2. **Week 3-4**: Version detection algorithm
3. **Week 5-6**: Migration engine for each version pair
4. **Week 7-8**: UI integration into beekeep
5. **Week 9-10**: Testing with real scenes from each version

**Features**:

- **Automatic Detection**: "This scene was created with BEES 0.7.2"
- **Conversion Options**: "Convert to BEES 1.0 format?"
- **Batch Processing**: Convert entire scene libraries
- **Backup Creation**: Preserves originals before conversion
- **Validation Reports**: Shows what changed during migration

**Integration with M1 Beekeep**:

- Build migration system into the working beekeep editor
- Provides immediate value for testing
- Creates comprehensive scene management tool

## Problem: M1 Mac Support

### Current Status on M1/M2/M3 Macs

**GTK Version (beekeep)**:

- ❌ Doesn't build natively for Apple Silicon
- ❌ GTK+ bundle incompatible with ARM64
- ❌ Rosetta 2 doesn't help (GTK framework issues)
- ❌ Homebrew/MacPorts GTK conflicts with bundle

**JUCE Version (beekeep_juce)**:

- ⚠️ JUCE itself supports M1/ARM64
- ❌ Beekeep_juce code incomplete
- ❌ Build system needs updating
- ❌ GUI never finished

### Why M1 Support is Important

**Market Reality**:

- All new Macs are Apple Silicon (since 2020)
- Intel Macs being phased out
- Developers use M1/M2/M3 Macs
- Without M1 support, beekeep is unusable for most

**Development Impact**:

- Can't test scenes without hardware
- Slow development workflow
- Barrier to new contributors
- Scene sharing more difficult

## Development Plan for Beekeep 1.0

### Approach 1: Fix GTK Version (Recommended)

**Rationale**:

- Working codebase exists
- Just needs packaging fixes
- Faster path to working editor

**Technical Plan**:

1. **Update GTK Bundle** (Week 1-2)

   - Build GTK+ 3 or 4 for ARM64
   - Create proper `.app` bundle for macOS
   - Include all dependencies
   - Fix icon/resource loading

2. **Code Fixes** (Week 2-3)

   - Add destroy-window handler (fixes segfault)
   - Fix file selection dialog
   - Improve error handling
   - Clean up command-line vs. GUI split

3. **Build System** (Week 3)

   - Update Makefiles for ARM64
   - CMake build system (optional)
   - GitHub Actions for auto-builds
   - Universal binary (Intel + ARM)

4. **Testing** (Week 4)
   - Test on M1/M2/M3 Macs
   - Verify scene compatibility
   - UI/UX improvements
   - Bug fixes

**Challenges**:

- GTK on macOS is notoriously difficult
- Bundle dependencies correctly
- May need to use gtk-mac-bundler
- Testing across macOS versions

**Deliverable**: Beekeep 1.0 for M1 Macs

- Native ARM64 binary
- Working .app bundle
- Stable scene editing
- Compatible with BEES 0.8.x / 1.0

### Approach 2: Complete JUCE Version (More Work)

**Rationale**:

- Modern, actively maintained framework
- Native M1 support built-in
- Better cross-platform support
- Cleaner architecture

**Technical Plan**:

1. **Assess Current Code** (Week 1)

   - Review beekeep_juce state
   - Identify what's missing
   - Check against latest JUCE
   - Compile and test

2. **Complete Implementation** (Weeks 2-6)

   - Finish GUI implementation
   - Operator node editor
   - Connection routing
   - Parameter editing
   - Scene save/load

3. **BEES Integration** (Week 7-8)

   - Scene format compatibility
   - Operator definitions
   - Parameter descriptors
   - Preset system

4. **Polish** (Weeks 9-10)
   - UI/UX improvements
   - Keyboard shortcuts
   - Undo/redo
   - Help system

**Challenges**:

- More work than fixing GTK version
- GUI needs to be built from scratch
- JUCE is large dependency
- Need JUCE expertise

**Deliverable**: Beekeep 2.0 (JUCE-based)

- Modern cross-platform editor
- Native on all platforms
- Better user experience
- Easier to maintain

### Approach 3: Web-Based Editor (Alternative)

**Rationale**:

- No platform issues
- Easy distribution
- Modern web tech stack
- Community can contribute

**Technical Plan**:

1. **Technology Stack** (Week 1)

   - React or Vue.js
   - Cytoscape.js (graph visualization)
   - Node.js backend (optional)
   - File API for local files

2. **Core Features** (Weeks 2-8)

   - Operator palette
   - Node graph editor
   - Connection routing
   - Parameter editing
   - Scene parser/generator

3. **BEES Integration** (Weeks 9-10)

   - Parse .scn files (JavaScript)
   - Generate .scn files
   - Validate scenes
   - Operator library

4. **Deployment** (Week 11-12)
   - Host on monome.org
   - GitHub Pages option
   - Offline PWA version
   - Desktop app (Electron)

**Challenges**:

- Different skill set required
- Browser file access limitations
- May need Electron for desktop feel
- No direct Aleph communication

**Deliverable**: Beekeep Web

- Browser-based editor
- Works on all platforms
- Easy updates
- Shareable patches via URL

## Updated Development Roadmap for BEES 1.0

### Immediate Next Steps (December 2025)

#### Phase 2A: Modern Grid Support (HIGH PRIORITY - 4 weeks)

**Scope**: Enable CDC-based monome grid compatibility

**Implementation**:

1. **Week 1**: Extract libavr32 updates from eurorack modules
2. **Week 2**: Integrate CDC USB support into Aleph firmware
3. **Week 3**: Test with FTDI and CDC grids, ensure compatibility
4. **Week 4**: Build optimized firmware with grid support

**Deliverables**:

- [ ] Updated libavr32 with CDC support
- [ ] Firmware supporting both FTDI and CDC grids
- [ ] Grid auto-detection functionality
- [ ] Tested with multiple grid models
- [ ] Updated build instructions

#### Phase 2B: M1 Beekeep + Scene Migration (8 weeks parallel)

**Scope**: Working beekeep on M1 Macs with version migration

**M1 Beekeep (Weeks 1-4)**:

1. **Week 1-2**: Build GTK for ARM64 / Fix .app bundle
2. **Week 3**: Test and fix bugs on M1/M2/M3 Macs
3. **Week 4**: Documentation and basic release

**Scene Migration System (Weeks 5-8)**:

1. **Week 5-6**: Scene format analysis and version detection
2. **Week 7**: Migration engine for 0.7 → 0.8/1.0 conversion
3. **Week 8**: UI integration and testing with real scenes

**Why This Approach**:

1. **Grid support unblocks hardware compatibility** (most critical)
2. **M1 beekeep enables desktop development** (workflow improvement)
3. **Scene migration enables testing with existing content** (development speed)
4. **All three work together** for comprehensive improvement

## Recommendation for BEES 1.0

### Phase 1 Approach: Integrated Development

**Scope**: Modern grid support + M1 beekeep + scene migration

**Why**:

1. **Grid support is essential** - most users have modern grids
2. **M1 beekeep unblocks desktop development**
3. **Scene migration enables testing with existing 0.7 content**
4. **Proven solutions exist** - libavr32 updates + GTK fixes
5. **Moderate effort, high impact**

**Timeline**: December 2025 - January 2026 (8 weeks parallel work)

### Combined Benefits

**Grid Support + M1 Beekeep + Scene Migration = Complete Development Environment**

1. **Hardware Compatibility**: Modern grids work with Aleph
2. **Desktop Development**: Fast patch creation on M1 Macs
3. **Content Migration**: Existing 0.7 scenes available for testing
4. **Rapid Iteration**: Desktop editing → hardware testing cycle
5. **Community Onboarding**: New users can use their hardware immediately

**Testing Strategy**:

- **Use migrated 0.7 scenes** to test new grid firmware
- **Create patches in beekeep** to test grid functionality
- **Validate scene migration** with real-world content
- **End-to-end workflow**: beekeep → scene file → Aleph → modern grid

**Deliverables**:

- [ ] Beekeep 1.0 for macOS ARM64
- [ ] .app bundle with all dependencies
- [ ] Tested on M1/M2/M3 Macs
- [ ] Scene compatibility with BEES 1.0
- [ ] Basic documentation

### Phase 2+ Options: Better Editor

After 1.0 release, consider:

- **Option A**: Complete JUCE version (if team has C++ expertise)
- **Option B**: Web-based editor (if team has web expertise)
- **Option C**: Keep improving GTK version

## Technical Details

### Scene File Format

**Format**: Plain text, custom format
**Extension**: `.scn`
**Contents**:

```
# BEES Scene File
VERSION: 0.8.x
OPERATORS: [list]
CONNECTIONS: [list]
PARAMETERS: [values]
PRESETS: [data]
```

**Compatibility**:

- 0.7.x format: Different (needs migration)
- 0.8.x format: Current
- 1.0 format: Will need updates

**Requirement**: Beekeep must handle version transitions

### Build Dependencies

#### GTK Version:

```bash
# macOS (Homebrew)
brew install gtk+3
brew install gtk-mac-bundler

# Build
cd utils/beekeep
make
```

#### JUCE Version:

```bash
# Get JUCE (already included?)
cd utils/beekeep_juce
# Open in Projucer or build with CMake
```

### M1-Specific Considerations

**Universal Binary**:

- Build for both Intel (x86_64) and ARM64
- Single .app works on all Macs
- Use `lipo` to combine

**Code Signing**:

- M1 Macs require signed apps (Gatekeeper)
- Need Apple Developer account
- Or distribute as source

**Testing**:

- M1 Mac Mini (dev machine)
- M2 MacBook (testing)
- Various macOS versions (Monterey+)

## Integration with BEES 1.0 Development

### Phase 2 (Months 3-4): Architecture Improvements

**Add Beekeep Work**:

- Week 1-2: Assess current beekeep state
- Week 2-3: Build GTK for ARM64 / Fix bundle
- Week 3: Test and fix bugs
- Week 4: Documentation and release

**Parallel Work**:

- Main BEES development continues
- Beekeep work doesn't block 1.0
- But provides huge UX improvement

**Success Criteria**:

- [ ] Builds natively on M1/M2/M3
- [ ] Loads 0.8.x scenes
- [ ] Saves scenes compatible with BEES 1.0
- [ ] No crashes on basic operations
- [ ] Basic docs for use

### Phase 3 (Months 5-6): Polish & Features

**Beekeep Improvements**:

- UI polish
- Better error messages
- Scene validation
- Example scene library
- Video tutorials

## Resources Needed

### Personnel:

- 1 developer with macOS/GTK experience
- Or: 1 developer with C++/JUCE experience
- Or: 1 web developer for web version

### Hardware:

- M1/M2/M3 Mac for testing
- Intel Mac for universal binary
- Aleph unit for testing scenes

### Time:

- GTK fix: 4 weeks (1 developer)
- JUCE completion: 10 weeks (1 developer)
- Web version: 12 weeks (1 developer)

## Alternative: Community Contribution

**Open Call**:
Since beekeep is open source, community could help:

1. **GitHub Issue**: "M1 Mac Support for Beekeep"
2. **Bounty**: Offer reward for working implementation
3. **Documentation**: Provide clear build instructions
4. **Testing**: Recruit M1 Mac users for testing

**Advantages**:

- Distributes work
- Faster progress
- Community engagement
- Multiple approaches possible

## Documentation Needs

### For Developers:

- beekeep architecture overview
- Build instructions (all platforms)
- Scene file format specification
- Contributing guide

### For Users:

- Installation guide
- Basic usage tutorial
- Keyboard shortcuts
- Troubleshooting

## Success Metrics

### For 1.0:

- [ ] Beekeep builds and runs on M1 Macs
- [ ] Can load 0.8.x scenes
- [ ] Can save scenes compatible with 1.0
- [ ] No major crashes
- [ ] 10+ users testing successfully

### For 2.0:

- [ ] Full featured editor
- [ ] Professional UI/UX
- [ ] Undo/redo working
- [ ] Scene library management
- [ ] Active user community

## Risks & Mitigation

**Risk**: GTK bundle issues persist

- **Mitigation**: Fall back to source-only distribution

**Risk**: No one with macOS/GTK expertise

- **Mitigation**: Web-based editor instead

**Risk**: JUCE version too much work

- **Mitigation**: Stick with GTK fix

**Risk**: Community doesn't contribute

- **Mitigation**: Hire contractor

## Long-term Vision

**Beekeep 2.0+**:

- Cloud sync for scenes
- Collaboration features
- Scene marketplace
- Integrated tutorials
- Video help system
- Live device connection
- Real-time parameter updates

**Integration with Teletype Editor**:

- Unified editor for BEES + Teletype
- Switch between modes
- Shared scene library
- Cross-mode routing

## Conclusion

Getting beekeep working on M1 Macs is **essential** for BEES 1.0 success. Without a desktop editor:

- Development is painful
- New users struggle
- Scene sharing is limited
- Community growth is stunted

**Recommended Path**:

1. Fix GTK version for M1 (fastest)
2. Include in BEES 1.0 release
3. Consider better editor post-1.0

This provides immediate value while leaving door open for better solutions later.

---

_Beekeep Development Plan v1.0_  
_For BEES 1.0 Development_  
_Priority: High (enables better workflow)_  
_Timeline: 4 weeks (parallel with Phase 2)_  
_November 2025_
