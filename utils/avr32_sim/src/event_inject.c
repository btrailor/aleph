/* event_inject.c
 * aleph / avr32_sim
 *
 * Implementation of the event injection API.
 * Packages arguments into the same data word layout used by the real
 * AVR32 interrupt handlers, then calls event_post().
 *
 * Only compiled as part of the simulator (avr32_sim).  Never included
 * in target firmware builds.
 */

#include "event_inject.h"
#include "event_types.h"
#include "events.h"

/* ---------------------------------------------------------------------------
 * Encoder events
 * kEventEncoder0..kEventEncoder3 carry a signed delta in data.
 * idx selects the event type; delta is cast straight to s32.
 * --------------------------------------------------------------------------*/
void inject_encoder(u8 idx, s32 delta) {
    event_t e;
    /* Clamp to valid range defensively; real hardware has 4 encoders. */
    if (idx > 3) { return; }
    e.type = (etype)(kEventEncoder0 + idx);
    e.data = (s32)delta;
    event_post(&e);
}

/* ---------------------------------------------------------------------------
 * Switch events
 * kEventSwitch0..kEventSwitch7 encode both index and state in data.
 * data = idx | (state << 8)
 * --------------------------------------------------------------------------*/
void inject_switch(u8 idx, u8 state) {
    event_t e;
    if (idx > 7) { return; }
    e.type = (etype)(kEventSwitch0 + idx);
    e.data = (s32)((u32)idx | ((u32)(state & 0x01) << 8));
    event_post(&e);
}

/* ---------------------------------------------------------------------------
 * ADC events
 * kEventAdc0..kEventAdc3 encode channel and 16-bit value in data.
 * data = ch | (value << 8)
 * --------------------------------------------------------------------------*/
void inject_adc(u8 ch, u16 value) {
    event_t e;
    if (ch > 3) { return; }
    e.type = (etype)(kEventAdc0 + ch);
    e.data = (s32)((u32)ch | ((u32)value << 8));
    event_post(&e);
}

/* ---------------------------------------------------------------------------
 * Monome grid key events
 * kEventMonomeGridKey encodes x, y, z in three consecutive bytes of data.
 * data = x | (y << 8) | (z << 16)
 * --------------------------------------------------------------------------*/
void inject_grid_key(u8 x, u8 y, u8 z) {
    event_t e;
    e.type = kEventMonomeGridKey;
    e.data = (s32)((u32)x | ((u32)y << 8) | ((u32)(z & 0x01) << 16));
    event_post(&e);
}

/* ---------------------------------------------------------------------------
 * MIDI packet events
 * kEventMidiPacket packs all four USB-MIDI bytes into data.
 * data = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24)
 * --------------------------------------------------------------------------*/
void inject_midi_packet(u8 b0, u8 b1, u8 b2, u8 b3) {
    event_t e;
    e.type = kEventMidiPacket;
    e.data = (s32)( (u32)b0
                  | ((u32)b1 << 8)
                  | ((u32)b2 << 16)
                  | ((u32)b3 << 24) );
    event_post(&e);
}

/* ---------------------------------------------------------------------------
 * Monome connect event
 * kEventMonomeConnect signals that a monome device has appeared on USB.
 * No payload; data = 0.
 * --------------------------------------------------------------------------*/
void inject_monome_connect(void) {
    event_t e;
    e.type = kEventMonomeConnect;
    e.data = 0;
    event_post(&e);
}

/* ---------------------------------------------------------------------------
 * MIDI connect event
 * kEventMidiConnect signals that a MIDI device has appeared on USB.
 * No payload; data = 0.
 * --------------------------------------------------------------------------*/
void inject_midi_connect(void) {
    event_t e;
    e.type = kEventMidiConnect;
    e.data = 0;
    event_post(&e);
}
