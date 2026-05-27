/* usb_sim.c
 * aleph / avr32_sim
 *
 * USB Simulation Layer — Phase 2 implementation.
 *
 * Provides event injection for monome connect/disconnect and MIDI,
 * plus LED buffer and MIDI output capture for test inspection.
 *
 * This version is self-contained and works with the existing codebase
 * without requiring modified monome.c / ftdi.c / midi.c.
 */

#include <string.h>
#include <stdio.h>

#include "usb_sim.h"
#include "event_inject.h"
#include "events.h"
#include "types.h"
#include "usb/ftdi/ftdi.h"
#include "usb/midi/midi.h"

/* -------------------------------------------------------------------------
 * LED buffer (local to usb_sim for test inspection)
 * --------------------------------------------------------------------------*/

/* 16x16 grid = 256 LEDs, one byte per LED (brightness 0-15) */
#define USB_SIM_LED_BUF_SIZE 256

static u8 usbSimLedBuffer[USB_SIM_LED_BUF_SIZE];
static u16 usbSimLedBufferLen = 0;
static u8 usbSimLedCols = 0;
static u8 usbSimLedRows = 0;

/* -------------------------------------------------------------------------
 * MIDI output buffer (shared with midi.c for test inspection)
 * --------------------------------------------------------------------------*/

extern midi_packet_t midiOutPackets[MIDI_OUT_PACKET_MAX];
extern u16 midiOutPacketCount;

/* -------------------------------------------------------------------------
 * Monome lifecycle
 * --------------------------------------------------------------------------*/

void usb_sim_monome_connect(u8 device, u8 cols, u8 rows) {
    event_t e;
    e.type = kEventMonomeConnect;
    /* Pack as four bytes little-endian: [device][cols][rows][0] */
    e.data = (s32)( (u32)device
                  | ((u32)cols  << 8)
                  | ((u32)rows  << 16) );
    fprintf(stderr, "[usb_sim] monome_connect device=%u cols=%u rows=%u\n",
            (unsigned)device, (unsigned)cols, (unsigned)rows);
    
    /* Store dimensions for LED buffer inspection */
    usbSimLedCols = cols;
    usbSimLedRows = rows;
    usbSimLedBufferLen = cols * rows;
    if (usbSimLedBufferLen > USB_SIM_LED_BUF_SIZE) {
        usbSimLedBufferLen = USB_SIM_LED_BUF_SIZE;
    }
    memset(usbSimLedBuffer, 0, USB_SIM_LED_BUF_SIZE);
    
    event_post(&e);
}

void usb_sim_monome_disconnect(void) {
    event_t e;
    e.type = kEventMonomeDisconnect;
    e.data = 0;
    fprintf(stderr, "[usb_sim] monome_disconnect\n");
    event_post(&e);
}

/* -------------------------------------------------------------------------
 * MIDI lifecycle
 * --------------------------------------------------------------------------*/

void usb_sim_midi_connect(void) {
    event_t e;
    e.type = kEventMidiConnect;
    e.data = 0;
    fprintf(stderr, "[usb_sim] midi_connect\n");
    event_post(&e);
}

void usb_sim_midi_disconnect(void) {
    event_t e;
    e.type = kEventMidiDisconnect;
    e.data = 0;
    fprintf(stderr, "[usb_sim] midi_disconnect\n");
    event_post(&e);
}

void usb_sim_midi_packet(u8 b0, u8 b1, u8 b2, u8 b3) {
    event_t e;
    e.type = kEventMidiPacket;
    /* Pack MIDI bytes into event data */
    e.data = (s32)( (u32)b0
                  | ((u32)b1 << 8)
                  | ((u32)b2 << 16)
                  | ((u32)b3 << 24) );
    fprintf(stderr, "[usb_sim] midi_packet 0x%02x 0x%02x 0x%02x 0x%02x\n",
            (unsigned)b0, (unsigned)b1, (unsigned)b2, (unsigned)b3);
    event_post(&e);
}

/* -------------------------------------------------------------------------
 * LED state inspection
 * --------------------------------------------------------------------------*/

int usb_sim_get_led_buffer(u8 *buf, u16 buf_capacity, u16 *len_out) {
    if (buf == NULL) {
        return 0;
    }
    if (buf_capacity < usbSimLedBufferLen) {
        return 0;
    }
    memcpy(buf, usbSimLedBuffer, usbSimLedBufferLen);
    if (len_out != NULL) {
        *len_out = usbSimLedBufferLen;
    }
    return 1;
}

u8 usb_sim_get_led_xy(u8 x, u8 y) {
    if (usbSimLedCols == 0 || usbSimLedRows == 0) {
        return 0;  /* No grid connected = all LEDs off */
    }
    if (x >= usbSimLedCols || y >= usbSimLedRows) {
        return 0xff;  /* Out of range */
    }
    return usbSimLedBuffer[y * usbSimLedCols + x];
}

/* -------------------------------------------------------------------------
 * MIDI output inspection
 * --------------------------------------------------------------------------*/

int usb_sim_get_midi_out_buffer(usb_sim_midi_packet_t *packets,
                                   u16 max_packets,
                                   u16 *count_out) {
    if (packets == NULL) {
        return 0;
    }
    u16 n = (midiOutPacketCount < max_packets) ? midiOutPacketCount : max_packets;
    memcpy(packets, midiOutPackets, n * sizeof(midi_packet_t));
    if (count_out != NULL) {
        *count_out = n;
    }
    return 1;
}

/* -------------------------------------------------------------------------
 * Capture outbound data (called by ftdi_write / uhi_midi_write stubs)
 * --------------------------------------------------------------------------*/

void usb_sim_capture_led(u8 x, u8 y, u8 val) {
    if (usbSimLedCols == 0 || usbSimLedRows == 0) {
        /* Default grid size for tests */
        usbSimLedCols = 16;
        usbSimLedRows = 8;
        usbSimLedBufferLen = usbSimLedCols * usbSimLedRows;
    }
    if (x >= usbSimLedCols || y >= usbSimLedRows) {
        return;
    }
    usbSimLedBuffer[y * usbSimLedCols + x] = val;
}

void usb_sim_capture_midi_out(u8 b0, u8 b1, u8 b2, u8 b3) {
    if (midiOutPacketCount >= MIDI_OUT_PACKET_MAX) {
        fprintf(stderr, "[usb_sim] midi out buffer full, dropping packet\n");
        return;
    }
    midiOutPackets[midiOutPacketCount].b[0] = b0;
    midiOutPackets[midiOutPacketCount].b[1] = b1;
    midiOutPackets[midiOutPacketCount].b[2] = b2;
    midiOutPackets[midiOutPacketCount].b[3] = b3;
    midiOutPacketCount++;
}

/* -------------------------------------------------------------------------
 * Reset all simulation state
 * --------------------------------------------------------------------------*/

void usb_sim_reset(void) {
    extern void init_events(void);
    memset(usbSimLedBuffer, 0, USB_SIM_LED_BUF_SIZE);
    usbSimLedBufferLen = 0;
    usbSimLedCols = 0;
    usbSimLedRows = 0;
    memset(midiOutPackets, 0, sizeof(midiOutPackets));
    midiOutPacketCount = 0;
    ftdiOutBufferLen = 0;
    memset(ftdiOutBuffer, 0, FTDI_OUT_BUF_SIZE);
    init_events();  /* clear event queue */
    fprintf(stderr, "[usb_sim] reset complete\n");
}
