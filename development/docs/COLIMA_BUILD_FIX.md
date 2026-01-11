# Colima Build Fix - File System Cache Issue

## Problem Description

When building Aleph firmware with Docker/Colima, you may encounter compilation errors where the compiler reports files as "shorter than expected":

```
warning: ../../common/module_common.h is shorter than expected
error: 'MODULE_NAME_LEN' undeclared here (not in a function)
```

When checking file line counts inside Docker, files show as 0 lines even though they exist on the host:

```bash
# Inside Docker container:
$ wc -l /aleph/common/module_common.h
0 /aleph/common/module_common.h

# On host system:
$ wc -l common/module_common.h
18 common/module_common.h
```

## Root Cause

This issue occurs when Colima's **sshfs mount** has stale file system cache. This can happen after:

- Directory renames or moves
- Extended periods without restarting Colima
- File system operations that don't properly sync with the VM

Colima uses sshfs (SSH File System) by default on macOS, which can cache file metadata and contents. When the cache becomes stale, Docker containers see empty or truncated files.

## Quick Fix

Restart Colima to clear the file system cache:

```bash
colima stop && colima start
```

Wait for Colima to fully start (about 10-15 seconds), then retry your build.

## Detailed Fix Process

### 1. Verify the Issue

Check if files are readable inside Docker:

```bash
docker run --rm -v "$(pwd):/aleph" aleph-builder:latest \
  bash -c "wc -l /aleph/common/module_common.h /aleph/common/param_common.h"
```

If this shows `0` lines for files that aren't empty on your host system, you have the cache issue.

### 2. Stop Colima

```bash
colima stop
```

This will shut down the Colima VM and clear any cached file system state.

### 3. Wait for Clean Shutdown

```bash
sleep 3
```

Give Colima a few seconds to fully shut down before restarting.

### 4. Start Colima

```bash
colima start
```

Wait for the startup process to complete. You should see:

```
INFO[0000] starting colima
INFO[0000] runtime: docker
INFO[0000] starting ...                                  context=vm
INFO[0012] provisioning ...                              context=docker
INFO[0013] starting ...                                  context=docker
INFO[0014] done
```

### 5. Verify the Fix

Re-check file readability:

```bash
docker run --rm -v "$(pwd):/aleph" aleph-builder:latest \
  bash -c "wc -l /aleph/common/module_common.h /aleph/common/param_common.h"
```

You should now see the correct line counts (e.g., `18` and `77`).

### 6. Rebuild

```bash
docker run --rm -v "$(pwd):/aleph" aleph-builder:latest \
  bash -c "cd /aleph/apps/bees && make clean && make"
```

The build should now complete successfully.

## One-Line Fix

For quick fixes, you can chain the entire process:

```bash
colima stop && sleep 3 && colima start && \
  docker run --rm -v "$(pwd):/aleph" aleph-builder:latest \
    bash -c "cd /aleph/apps/bees && make clean && make"
```

## Prevention

To minimize the likelihood of this issue:

1. **Regular restarts**: Restart Colima periodically, especially after major file system operations
2. **Avoid directory renames**: Try to establish your project directory structure early
3. **Check Colima status**: Run `colima status` to verify VM health before builds

## Checking Your Colima Configuration

To see your current Colima setup:

```bash
colima status
```

Key information:

- **mountType**: Should show `sshfs` (this is where caching issues occur)
- **runtime**: Should show `docker`
- **arch**: Should show `aarch64` on Apple Silicon

Example output:

```
INFO[0000] colima is running using macOS Virtualization.Framework
INFO[0000] arch: aarch64
INFO[0000] runtime: docker
INFO[0000] mountType: sshfs
INFO[0000] socket: unix:///Users/username/.colima/default/docker.sock
```

## Alternative: Using VirtioFS (Advanced)

If you frequently encounter this issue, consider switching to VirtioFS instead of sshfs:

```bash
colima stop
colima start --mount-type virtiofs
```

VirtioFS has better performance and more reliable caching, but may require additional setup.

## When This Fix Doesn't Work

If restarting Colima doesn't resolve the issue:

1. **Full rebuild of Colima VM**:

   ```bash
   colima delete
   colima start
   ```

   ⚠️ Warning: This will delete your VM and all containers

2. **Check Docker image**:

   ```bash
   docker images aleph-builder
   ```

   Verify the image exists and was created successfully

3. **Check file permissions**:
   ```bash
   ls -la /Users/your-username/Documents/01_Projects/Aleph/common/
   ```
   Ensure files are readable (should have `r` permission)

## Related Issues

- Docker "platform mismatch" warning is normal on Apple Silicon and doesn't affect functionality
- Compilation warnings about "volatile" or "incompatible pointer types" are expected and non-fatal
- Missing Docker Desktop plugins warnings can be ignored if using Colima

## Build History Note

This issue was diagnosed on November 12, 2025, when firmware built successfully on November 11 at 23:06 but failed afterward despite no code changes. The root cause was Colima's sshfs cache becoming stale overnight.
