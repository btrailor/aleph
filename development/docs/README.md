# Aleph BEES 1.0 Development - Documentation Index

## Complete Documentation Package

This is your complete roadmap for developing BEES 1.0, based on the discovery that BEES 0.8.x (unofficial, 2018) represents the actual current state of the project.

---

## 📋 Core Documents

### 1. **Main Development Pathway** (v2.0) ⭐ START HERE
**File**: `aleph-bees-1.0-development-pathway.md`  
**Purpose**: Complete roadmap from 0.8.x foundation to 1.0 release  
**Length**: ~2000 lines  
**Updated**: November 2025

**Contains**:
- Current state analysis (0.8.x reality)
- Critical issues (solved vs. still open)
- Development phases (0-4, 8-9 months)
- Breaking changes policy
- Success metrics
- Complete timeline

**Key Sections**:
- Phase 0: Verify 0.8.x baseline (NEW)
- Issues already solved in 0.8.x ✅
- Building on 0.8.x foundation
- Teletype port (bonus goal)
- Audio FX system (long-term goal)
- **Beekeep desktop editor (NEW)** ⚠️

---

## 🔍 Supporting Documents

### 2. **BEES 0.8.x Analysis**
**File**: `bees-0.8-analysis.md`  
**Purpose**: Deep dive into what 0.8.x accomplished  
**Key Info**: What was solved, technical details, lessons learned

**Highlights**:
- Dynamic memory management (PR #267)
- CV output fixes (Issue #294)
- Preset system overhaul
- Community history (boqs and contributors)
- Technical debt inherited

### 3. **Development Pathway Update Summary**
**File**: `development-pathway-update-summary.md`  
**Purpose**: Quick overview of what changed after discovering 0.8.x  
**Format**: Before/after comparison

**Shows**:
- Original plan vs. revised plan
- What 0.8.x solved
- New timeline
- Why this matters

### 4. **Quick Start Guide**
**File**: `quick-start-guide.md`  
**Purpose**: Practical guide to start development  
**Audience**: Developers ready to begin work

**Contains**:
- How to clone the RIGHT branch (dev, not master!)
- Toolchain setup
- Phase 0 tasks (week by week)
- Testing procedures
- Common gotchas

### 5. **Beekeep Development Plan** (NEW)
**File**: `beekeep-development-plan.md`  
**Purpose**: Complete plan for desktop editor  
**Focus**: Getting it working on M1/M2/M3 Macs

**Covers**:
- Current state (broken on Apple Silicon)
- Three approaches (GTK fix, JUCE, Web)
- Recommended path (GTK fix - 4 weeks)
- Integration with BEES 1.0
- Why this is critical

---

## 📊 Quick Reference

### Critical Discoveries

1. **0.8.x is the real current version** (not 0.7.1)
   - Located on `dev` branch
   - Released January 2018
   - Solves major issues

2. **Memory management already solved** ✅
   - PR #267 implemented
   - Dynamic operators work

3. **CV glitches already fixed** ✅
   - Issue #294 resolved
   - Professional reliability

4. **Preset system working** ✅
   - Was broken in 0.7.x
   - Fixed in 0.8.x

5. **Beekeep broken on modern Macs** ⚠️
   - Doesn't work on M1/M2/M3
   - Needs urgent attention
   - Critical for development

### Development Timeline

```
Month 0:    Phase 0 - Verify 0.8.x baseline
Month 1-2:  Phase 1 - Stabilization
Month 3-4:  Phase 2 - Architecture + Beekeep fix
Month 5-6:  Phase 3 - Polish + Start Teletype
Month 7-8:  Phase 4 - Testing & Release
Month 9:    BEES 1.0 Release ✅
Month 9-12: Complete Teletype port
Month 12:   Teletype 1.0 Release ✅
Year 2:     Audio FX system
```

### Key Decisions Made

1. ✅ Start from `dev` branch (0.8.x), NOT `master` (0.7.1)
2. ✅ Preserve all 0.8.x improvements
3. ✅ Add Phase 0 for baseline verification
4. ✅ Include beekeep M1 fix in Phase 2
5. ✅ Breaking changes acceptable (0.8.x already broke compatibility)
6. ✅ Migration tools for 0.7.x and 0.8.x users
7. ✅ Honor community contributions (boqs, catfact, etc.)

### Priority Matrix

**Must Have for 1.0**:
- ✅ All 0.8.x features preserved
- ✅ Input/output refactor completed
- ✅ SD card hot-swap fixed
- ✅ Beekeep working on M1 Macs
- ✅ Scene migration tools
- ✅ Comprehensive documentation

**Nice to Have for 1.0**:
- ⭐ Multiple monome support
- ⭐ Complete I2C handler
- ⭐ Time parameter improvements
- ⭐ New operators

**Post-1.0**:
- 🎯 Teletype port (months 5-12)
- 🎯 Audio FX system (year 2)
- 🎯 Web-based beekeep
- 🎯 Advanced features

---

## 🚀 Getting Started

### Step 1: Read Core Documents
1. Start with **Main Development Pathway** (comprehensive overview)
2. Read **0.8.x Analysis** (understand what's already done)
3. Review **Update Summary** (see what changed)

### Step 2: Practical Setup
1. Follow **Quick Start Guide** (hands-on instructions)
2. Clone from `dev` branch
3. Build 0.8.x
4. Verify baseline

### Step 3: Plan Your Work
1. Review **Beekeep Plan** (if working on desktop editor)
2. Choose development phase to tackle
3. Contact community (lines forum)
4. Start coding!

---

## 📁 File Organization

```
/mnt/user-data/outputs/
├── aleph-bees-1.0-development-pathway.md  ⭐ MAIN DOCUMENT
├── bees-0.8-analysis.md
├── development-pathway-update-summary.md
├── quick-start-guide.md
└── beekeep-development-plan.md            🆕 NEW
```

---

## 🎯 Success Criteria

### Phase 0 Complete:
- [ ] 0.8.x builds from `dev` branch
- [ ] All features tested on hardware
- [ ] Baseline documented
- [ ] Team understands codebase

### BEES 1.0 Complete:
- [ ] All 0.8.x improvements preserved
- [ ] Critical issues resolved
- [ ] Beekeep working on M1 Macs
- [ ] Migration tools functional
- [ ] Documentation comprehensive
- [ ] Community engaged
- [ ] Official release with binaries

### Bonus Goals:
- [ ] Teletype port complete (1.0)
- [ ] Audio FX system spec'd (roadmap)
- [ ] Web-based beekeep (future)

---

## 👥 Contributors & Community

**Original 0.8.x Team**:
- **boqs**: Dynamic memory, CV fixes, majority of 0.8.x work
- **catfact**: Core BEES development, DSP modules
- **tehn**: Original vision, ongoing guidance
- **rick-monster**: Network refactoring experiments

**Community**:
- lines forum (llllllll.co): Active discussion
- GitHub: Issues and pull requests
- Users: Beta testing and feedback

**1.0 Development**:
- You! Building on this foundation
- New contributors welcome
- See contribution guidelines

---

## 📞 Contact & Resources

**Official**:
- Docs: http://monome.org/docs/aleph
- GitHub: https://github.com/monome/aleph
- Support: help@monome.org

**Community**:
- Forum: https://llllllll.co
- Issues: https://github.com/monome/aleph/issues

**Key Branches**:
- `dev`: 0.8.x code ⭐ USE THIS
- `master`: 0.7.1 code (old, don't use)
- Forks: Check boqs, catfact, rick-monster

---

## 🔄 Document Version Control

**Version 2.0** (Current)
- Added BEES 0.8.x foundation
- Revised entire development pathway
- Added Phase 0 verification
- Included beekeep M1 Mac support
- Updated timelines and priorities

**Version 1.1** (Previous)
- Added Teletype port
- Added audio FX system

**Version 1.0** (Original)
- Initial development pathway
- Started from 0.7.1 (incorrect)

---

## ⚠️ Critical Warnings

1. **DO NOT use `master` branch** - it has old 0.7.1 code
2. **DO use `dev` branch** - it has working 0.8.x code
3. **Preserve 0.8.x improvements** - don't re-solve solved problems
4. **Test on hardware** - emulation isn't enough
5. **Engage community** - they've kept this alive

---

## 💡 Key Insights

**Before 0.8.x Discovery**:
- Would have spent months re-solving problems
- Would have ignored 4 years of community work
- Would have started from inferior codebase
- Timeline would have been much longer

**After 0.8.x Discovery**:
- Build on solid, tested foundation
- Honor community contributions
- Faster path to 1.0
- Better end result
- Beekeep identified as critical need

**Bottom Line**:
BEES 1.0 is not building from scratch - it's **completing and officiating** what the community started with 0.8.x, plus adding new capabilities (beekeep M1, Teletype, FX).

---

*Documentation Index v1.0*  
*Complete Package for BEES 1.0 Development*  
*Foundation: BEES 0.8.x (dev branch)*  
*November 2025*
