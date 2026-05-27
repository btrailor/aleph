#ifndef _USB_MIDI_H_
#define _USB_MIDI_H_

#include "types.h"
//#include "uhc.h"

extern u8 midiConnect;

// read and spawn events (non-blocking)
extern void midi_read(void);

// write to MIDI device
extern void midi_write(u8* data, u32 bytes);

// MIDI device was plugged or unplugged
// extern void midi_change(uhc_device_t* dev, u8 plug);

// main-loop setup routine for new device connection.
// this is the place to perform any queries which require interrupts,
// and whose completion is necessary before polling.
// extern void midi_setup(void);

/* ---- Simulation additions for Phase 2 ---- */
#define MIDI_OUT_PACKET_MAX 256

typedef struct {
    u8 b[4];
} midi_packet_t;

extern midi_packet_t midiOutPackets[MIDI_OUT_PACKET_MAX];
extern u16 midiOutPacketCount;

extern void uhi_midi_write(u8 b0, u8 b1, u8 b2, u8 b3);
extern void midi_out_buf_clear(void);

#endif
