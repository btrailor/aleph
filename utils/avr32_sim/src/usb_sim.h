/* usb_sim.h
 * aleph / avr32_sim
 *
 * USB Simulation Layer — Phase 2.
 *
 * Provides a high-level test API for connecting/disconnecting USB devices
 * and inspecting the state they produce.  Wraps the lower-level
 * event_inject.h and the FTDI/MIDI capture buffers.
 *
 * Design intent
 * =============
 * Test code should not need to know about event encoding or buffer layout.
 * usb_sim gives it three things:
 *
 *   1. Device lifecycle — connect/disconnect events that BEES reacts to.
 *   2. Input injection  — grid key presses, MIDI packets.
 *   3. Output inspection — what LED state / MIDI bytes did BEES produce.
 *
 * Only include this header in test code and simulator harness code.
 * Never in BEES app logic.
 */

#ifndef _USB_SIM_H_
#define _USB_SIM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/* ==========================================================================
 * Monome / grid lifecycle (with transport distinction)
 * ========================================================================== */

/*
 * Transport type for connected monome device.
 * Tracks whether the emulator is using FTDI or CDC transport.
 */
typedef enum {
  eUsbSimTransportNone = 0,
  eUsbSimTransportFTDI,
  eUsbSimTransportCDC
} eUsbSimTransport;

/*
 * usb_sim_monome_connect_ftdi — connect a legacy FTDI-based grid.
 * Posts kEventMonomeConnect and sets transport to FTDI.
 */
void usb_sim_monome_connect_ftdi(u8 device, u8 cols, u8 rows);

/*
 * usb_sim_monome_connect_cdc — connect a modern CDC-based grid.
 * Posts kEventMonomeConnect and sets transport to CDC.
 */
void usb_sim_monome_connect_cdc(u8 device, u8 cols, u8 rows);

/*
 * usb_sim_monome_connect — legacy alias, defaults to FTDI for backward compat.
 * DEPRECATED: use usb_sim_monome_connect_ftdi() or usb_sim_monome_connect_cdc().
 */
void usb_sim_monome_connect(u8 device, u8 cols, u8 rows);

/*
 * usb_sim_get_transport — return the current transport type.
 * Returns eUsbSimTransportNone if no device is connected.
 */
eUsbSimTransport usb_sim_get_transport(void);

/*
 * usb_sim_monome_disconnect — post kEventMonomeDisconnect.
 * data = 0. Resets transport to None.
 */
void usb_sim_monome_disconnect(void);

/* ==========================================================================
 * MIDI lifecycle
 * ========================================================================== */

/*
 * usb_sim_midi_connect — post kEventMidiConnect.
 * data = 0.
 */
void usb_sim_midi_connect(void);

/*
 * usb_sim_midi_disconnect — post kEventMidiDisconnect.
 * data = 0.
 */
void usb_sim_midi_disconnect(void);

/*
 * usb_sim_midi_packet — post kEventMidiPacket.
 *
 * b0–b3 : raw USB MIDI packet bytes (CIN, status, data1, data2).
 *
 * Encodes as b0 | (b1 << 8) | (b2 << 16) | (b3 << 24) in event.data.
 */
void usb_sim_midi_packet(u8 b0, u8 b1, u8 b2, u8 b3);

/* ==========================================================================
 * LED state inspection
 * ========================================================================== */

/*
 * usb_sim_get_led_buffer — copy the current LED state into caller's buffer.
 *
 * Calls monome_led_flush() to serialise the grid first, then copies
 * monomeLedBuffer into buf (up to buf_capacity bytes).
 * Sets *len_out to the number of bytes written (= GRID_ROWS × GRID_COLS).
 *
 * buf          : destination buffer (must be at least GRID_ROWS*GRID_COLS bytes)
 * buf_capacity : size of buf in bytes
 * len_out      : set to number of bytes actually written; may be NULL
 *
 * Returns 1 on success, 0 if buf is too small.
 */
int usb_sim_get_led_buffer(u8 *buf, u16 buf_capacity, u16 *len_out);

/*
 * usb_sim_get_led_xy — return the brightness of a single grid LED.
 *
 * Calls monome_led_flush() to ensure the buffer is current.
 * x, y : grid coordinates (0-based)
 *
 * Returns brightness 0–15, or 0xff on out-of-range coordinates.
 */
u8 usb_sim_get_led_xy(u8 x, u8 y);

/* ==========================================================================
 * MIDI output inspection
 * ========================================================================== */

/*
 * usb_sim_get_midi_out_buffer — copy captured outbound MIDI packets.
 *
 * packets     : destination array (must hold at least max_packets entries)
 * max_packets : capacity of the destination array
 * count_out   : set to the number of packets copied; may be NULL
 *
 * Returns 1 on success, 0 if packets is NULL.
 */
typedef struct {
    u8 b[4];
} usb_sim_midi_packet_t;

int usb_sim_get_midi_out_buffer(usb_sim_midi_packet_t *packets,
                                 u16 max_packets,
                                 u16 *count_out);

/* ==========================================================================
 * Reset helpers
 * ========================================================================== */

/*
 * usb_sim_reset — clear all capture buffers and the event queue.
 * Call between test cases to start from a clean state.
 */
void usb_sim_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* _USB_SIM_H_ */
