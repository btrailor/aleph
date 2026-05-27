/* event_inject.h
 * aleph / avr32_sim
 *
 * API for injecting simulated hardware events into the event queue.
 * Used exclusively by test harnesses and simulator scaffolding —
 * never included in real firmware or BEES app code.
 *
 * Each function packages its arguments into the data word layout used
 * by the real AVR32 interrupt handlers (see interrupts.c), then calls
 * event_post() so the app event loop processes them identically to
 * real hardware events.
 */

#ifndef _EVENT_INJECT_H_
#define _EVENT_INJECT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/*
 * inject_encoder — simulate a relative encoder turn.
 *
 * idx   : encoder index 0–3  (maps to kEventEncoder0..kEventEncoder3)
 * delta : signed tick count  (positive = CW, negative = CCW)
 *
 * data encoding: data = (s32)delta
 */
void inject_encoder(u8 idx, s32 delta);

/*
 * inject_switch — simulate a front-panel switch press or release.
 *
 * idx   : switch index 0–7  (maps to kEventSwitch0..kEventSwitch7)
 * state : 1 = pressed, 0 = released
 *
 * data encoding: data = idx | (state << 8)
 */
void inject_switch(u8 idx, u8 state);

/*
 * inject_adc — simulate an ADC channel reading.
 *
 * ch    : ADC channel 0–3  (maps to kEventAdc0..kEventAdc3)
 * value : 12-bit ADC value (0–4095 typical)
 *
 * data encoding: data = ch | (value << 8)
 */
void inject_adc(u8 ch, u16 value);

/*
 * inject_grid_key — simulate a monome grid key press or release.
 *
 * x, y  : grid coordinates
 * z     : 1 = key down, 0 = key up
 *
 * data encoding: data = x | (y << 8) | (z << 16)
 */
void inject_grid_key(u8 x, u8 y, u8 z);

/*
 * inject_midi_packet — simulate a four-byte USB MIDI packet.
 *
 * b0–b3 : raw MIDI packet bytes (USB-MIDI packet format)
 *
 * data encoding: data = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24)
 */
void inject_midi_packet(u8 b0, u8 b1, u8 b2, u8 b3);

/*
 * inject_monome_connect — simulate a monome device connecting over USB.
 *
 * Posts kEventMonomeConnect with data = 0.
 */
void inject_monome_connect(void);

/*
 * inject_midi_connect — simulate a MIDI device connecting over USB.
 *
 * Posts kEventMidiConnect with data = 0.
 */
void inject_midi_connect(void);

#ifdef __cplusplus
}
#endif

#endif /* _EVENT_INJECT_H_ */
