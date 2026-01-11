# Development Daily Checklist

Quick reference for stable Aleph development workflow.

---

## Before Starting Work

```bash
# 1. Verify branch
git branch  # Should be on 'develop' or 'feature/*'

# 2. Pull latest
git pull origin develop

# 3. Check libavr32 submodule
cd libavr32
git branch  # Should show: * aleph-cdc-compat
git log -1  # Should show: 02469a2 Fix aleph build compatibility (or newer)
git status  # Should be: nothing to commit, working tree clean
cd ..

# 4. If libavr32 is wrong
cd libavr32
git checkout aleph-cdc-compat
git pull origin aleph-cdc-compat
cd ..
git submodule update --init
```

**✅ Ready to work when**:

- On correct branch
- libavr32 on aleph-cdc-compat
- No unexpected uncommitted changes

---

## During Development

### Rule 1: Commit libavr32 Changes Immediately

```bash
# After ANY change to files in libavr32/:
cd libavr32
git status
git add .
git commit -m "Fix: [what you fixed]"
git push origin aleph-cdc-compat
cd ..
git add libavr32
git commit -m "Update libavr32: [what changed]"
```

**⚠️ Never leave libavr32 changes uncommitted overnight!**

### Rule 2: Commit Often

```bash
# After each logical change:
git add [files]
git commit -m "[Area]: [what changed]"

# Examples:
git commit -m "Param: Add debug logging to unpickle functions"
git commit -m "Scene: Implement conversion from v0.7.1"
git commit -m "Docs: Update build guide with Docker issue"
```

### Rule 3: Test Before Committing

```bash
# 1. Build (when Docker is working)
docker run --rm -v "$(pwd):/host" -w /tmp aleph-builder bash -c \
  "cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && make"

# 2. Check for errors
# 3. If build succeeds:
git add -A
git commit -m "..."
```

---

## Building (When Docker Works)

### Quick Build

```bash
cd /path/to/aleph
docker run --rm -v "$(pwd):/host" -w /tmp aleph-builder bash -c \
  "cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && make && \
   cp aleph-bees.hex aleph-bees.elf /host/apps/bees/"
```

### Verify Build

```bash
ls -lh apps/bees/aleph-bees.hex
# Should be ~480-490 KB
```

---

## Ending Work Session

```bash
# 1. Check for uncommitted changes
git status

# 2. Commit or stash
git add -A
git commit -m "WIP: [description]"
# OR
git stash push -m "WIP $(date +%Y-%m-%d): [description]"

# 3. Push to remote (backup)
git push origin [your-branch]

# 4. Verify libavr32 is committed
cd libavr32
git status  # Should be clean
cd ..
```

---

## Common Issues

### "libavr32 has uncommitted changes"

```bash
cd libavr32
git status
# If you want to keep changes:
git add .
git commit -m "Fix: ..."
git push origin aleph-cdc-compat
cd ..
git add libavr32
git commit -m "Update libavr32"

# If you want to discard:
git checkout .
```

### "Docker says 'empty file'"

Your Docker version has volume mount bug. Solutions:

1. Update Docker Desktop to 24.x+
2. Use Jan 10 hex file as temporary workaround
3. See DEVELOPMENT_STABILITY_STRATEGY.md

### "Build used to work, now fails"

```bash
# Check what changed
git log -5 --oneline

# Check libavr32
cd libavr32
git log -5 --oneline
git status

# Reset to last working state
git checkout [last-working-commit]
cd libavr32
git checkout aleph-cdc-compat
```

### "Can't switch branches"

```bash
# Commit or stash changes first
git status
git stash push -m "Temp stash before branch switch"
git checkout [other-branch]

# After switching, restore if needed
git stash pop
```

---

## Emergency: Start Fresh

```bash
# 1. Save your work
git stash push -m "Emergency backup $(date +%Y-%m-%d)"

# 2. Reset to develop
git checkout develop
git reset --hard origin/develop

# 3. Reset libavr32
cd libavr32
git checkout aleph-cdc-compat
git reset --hard origin/aleph-cdc-compat
cd ..
git submodule update --init

# 4. Verify
git status  # Should be clean
cd libavr32 && git status && cd ..  # Should be clean

# 5. Restore your work
git stash list
git stash pop
```

---

## Success Checklist

Before pushing or ending session:

- [ ] All changes committed (no "modified" files in `git status`)
- [ ] libavr32 changes committed to aleph-cdc-compat
- [ ] Build succeeds (or Docker issue documented as blocker)
- [ ] Commit messages are clear
- [ ] Pushed to remote (backup)

---

## Quick Reference

| Command                     | Purpose             |
| --------------------------- | ------------------- |
| `git status`                | See what changed    |
| `git add -A`                | Stage all changes   |
| `git commit -m "..."`       | Commit with message |
| `git push`                  | Upload to GitHub    |
| `git stash`                 | Temporary save      |
| `cd libavr32 && git status` | Check submodule     |
| `git log --oneline -10`     | Recent commits      |
| `git diff`                  | See changes         |

---

**Remember**: Commit libavr32 changes immediately, commit often, never leave repo dirty overnight.
