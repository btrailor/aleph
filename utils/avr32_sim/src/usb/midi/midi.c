/* midi.c
 * aleph / avr32_sim / usb/midi
 *
 * USB MIDI stub — outbound packet logger.
 *
 * Real behaviour: uhi_midi_write() queues a 4-byte USB MIDI packet for
 * transmission to the attached MIDI device.
 *
 * Sim behaviour: appends the packet to midiOutPackets so tests can
 * assert on BEES MIDI output without real hardware.
 *
 * Wire-up: BEES calls uhi_midi_write() from net_midi.c and op_midi_out_note.c.
 * usb_sim_get_midi_out_buffer() exposes the captured packets to the test layer.
 */

#include <string.h>
#include <stdio.h>

#include "midi.h"

/* -------------------------------------------------------------------------
 * Capture state
 * --------------------------------------------------------------------------*/
midi_packet_t midiOutPackets[MIDI_OUT_PACKET_MAX];
u16           midiOutPacketCount = 0;

/* -------------------------------------------------------------------------
 * uhi_midi_write
 * --------------------------------------------------------------------------*/
void uhi_midi_write(u8 b0, u8 b1, u8 b2, u8 b3) {
    extern void usb_sim_capture_midi_out(u8 b0, u8 b1, u8 b2, u8 b3);
    usb_sim_capture_midi_out(b0, b1, b2, b3);
}

void midi_out_buf_clear(void) {
    extern u16 midiOutPacketCount;
    midiOutPacketCount = 0;
    memset(midiOutPackets, 0, sizeof(midiOutPackets));
}

/* -------------------------------------------------------------------------
 * Stubs for functions declared in midi.h but not implemented
 * --------------------------------------------------------------------------*/

void midi_read(void) { }

void midi_write(u8* data, u32 bytes) { }
