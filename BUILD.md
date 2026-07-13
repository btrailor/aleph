# Building Aleph BEES Firmware

Verified: 2026-07-13. Any machine with Docker Desktop (macOS/Linux) and the `aleph-builder` image can reproduce a flashable BEES hex from a clean GitHub clone.

## One-shot fresh-clone build

```bash
git clone --branch develop https://github.com/btrailor/aleph.git
cd aleph
git submodule update --init

# Requires the aleph-builder Docker image (see "Docker image" below)
docker run --rm --platform linux/amd64 \
  -v "$(pwd):/host" -w /tmp \
  aleph-builder:latest bash -c \
  'cp -r /host /tmp/aleph && cd /tmp/aleph/apps/bees && export PATH=/root/avr32-toolchain-linux/bin:$PATH && make && cp aleph-bees.hex aleph-bees.elf /host/apps/bees/'
```

Output: `apps/bees/aleph-bees.hex` (flashable firmware) and `apps/bees/aleph-bees.elf`.

## Why the copy to `/tmp`?

The AVR32 build writes many small object/dependency files. On macOS arm64 with Docker Desktop running amd64 emulation, writing these files back to a bind-mounted host volume is slow and can silently produce empty files. Building in the container's `/tmp` and copying only the final artifacts back avoids the problem entirely.

## Docker image

The build depends on the `aleph-builder` image, which contains the AVR32 GCC toolchain and ASF sources.

- **If you already have it**: `docker images | grep aleph-builder`
- **If you need to build it locally** (one-time, ~15–30 min):
  ```bash
  cd development/docker
  docker build -t aleph-builder .
  ```
- **If you have a backup tar**: `docker load -i aleph-builder-backup.tar.gz`

## What changed (2026-07-13)

The `develop` branch now builds reproducibly from a fresh clone. Prior blockers that were fixed:

- `.gitmodules` pointed to `monome/libavr32` at commit `02469a2`, which was no longer fetchable. It now points to `btrailor/libavr32` branch `cdc-transport` at a reachable commit.
- `apps/aleph_avr32_src.mk` had a duplicate `monome_transport.c`, was missing `adc.c`, and was missing the CDC class include path.
- `apps/bees/src/param.c` referenced `pnode->idx`, but `pnode_t` no longer has that field.
- `libavr32/src/usb/cdc/cdc.c` was missing `cdc_disconnect()`, which `avr32/src/main.c` expects.

## Verification

A successful build ends with:

```
OBJCOPY aleph-bees.hex
OBJCOPY aleph-bees.bin
```

and produces:

```bash
$ ls -lh apps/bees/aleph-bees.hex
-rw-r--r--  1 user  group  490K Jul 13 11:03 apps/bees/aleph-bees.hex
```

The hex is ready to copy to the Aleph SD card `app/` folder or flash with an AVR32 programmer.

## Branch notes

- Use branch `develop` for current work.
- Do **not** use `main` for builds until Phase A is extended to it; `main` still pins an older libavr32 state that is missing the CDC transport files.

## Flashing

Copy the hex to the Aleph SD card:

```bash
cp apps/bees/aleph-bees.hex /Volumes/ALEPH/app/
```

or place it in the `app/` folder of the SD card and power-cycle the Aleph.

## Scene-load sanity check

After flashing, load a **scene created on this firmware** (not a legacy v0.7.x scene) and confirm the parameter network does not scramble. Legacy grid scenes may mis-wire due to operator input/output count drift across the 0.7→0.8 transition; that migration is tracked separately.
