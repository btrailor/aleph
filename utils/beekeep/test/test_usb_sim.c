/* test_usb_sim.c
 * utils/beekeep/test
 *
 * USB Simulation Layer Test Suite — Phase 2.
 *
 * Tests the usb_sim.c API: event injection, LED buffer inspection,
 * MIDI output capture.  These tests exercise the sim layer in isolation
 * without BEES app logic — they verify the plumbing, not BEES behaviour.
 *
 * Test inventory
 * ==============
 *   U01  usb_sim_reset clears the event queue
 *   U02  usb_sim_monome_connect posts kEventMonomeConnect
 *   U03  kEventMonomeConnect data encodes device/cols/rows correctly
 *   U04  usb_sim_monome_disconnect posts kEventMonomeDisconnect
 *   U05  usb_sim_midi_connect posts kEventMidiConnect
 *   U06  usb_sim_midi_disconnect posts kEventMidiDisconnect
 *   U07  usb_sim_midi_packet posts kEventMidiPacket with correct encoding
 *   U08  LED buffer all-zero after reset
 *   U09  monome_led_set is visible via usb_sim_get_led_buffer
 *   U10  usb_sim_get_led_xy returns correct single-cell brightness
 *   U11  monome_led_all fills the entire buffer
 *   U12  monome_led_map sets a quadrant correctly
 *   U13  LED buffer reset between test cases via usb_sim_reset
 *   U14  MIDI out buffer empty after reset
 *   U15  uhi_midi_write is visible via usb_sim_get_midi_out_buffer
 *   U16  multiple MIDI packets captured in order
 *   U17  MIDI out buffer reset between test cases via usb_sim_reset
 *   U18  ftdi_write captured in ftdiOutBuffer
 *   U19  ftdi_out_buf_clear resets the FTDI buffer
 *   U20  usb_sim_reset clears FTDI + MIDI buffers atomically
 */

#include <stdio.h>
#include <string.h>

#include "usb_sim.h"
#include "events.h"
#include "event_types.h"
#include "monome.h"
#include "monome_transport_sim.h"
#include "usb/ftdi/ftdi.h"
#include "usb/cdc/cdc_sim.h"
#include "usb/midi/midi.h"
#include "types.h"

/* -------------------------------------------------------------------------
 * TAP harness (same as test_scene_migration.c)
 * --------------------------------------------------------------------------*/
static int _test_count = 0;
static int _fail_count = 0;

#define ASSERT_EQ(a, b, desc) \
    do { \
        _test_count++; \
        if ((int)(a) == (int)(b)) { \
            printf("ok %d - %s\n", _test_count, desc); \
        } else { \
            printf("not ok %d - %s (got %d, expected %d)\n", \
                   _test_count, desc, (int)(a), (int)(b)); \
            _fail_count++; \
        } \
    } while(0)

#define ASSERT_NEQ(a, b, desc) \
    do { \
        _test_count++; \
        if ((int)(a) != (int)(b)) { \
            printf("ok %d - %s\n", _test_count, desc); \
        } else { \
            printf("not ok %d - %s (got %d, did not expect it)\n", \
                   _test_count, desc, (int)(a)); \
            _fail_count++; \
        } \
    } while(0)

/* -------------------------------------------------------------------------
 * U01  usb_sim_reset clears the event queue
 * --------------------------------------------------------------------------*/
static void test_reset_clears_queue(void) {
    event_t e;
    e.type = kEventMonomeConnect;
    e.data = 0;
    event_post(&e);
    usb_sim_reset();
    ASSERT_EQ(event_queue_count(), 0, "U01: usb_sim_reset clears event queue");
}

/* -------------------------------------------------------------------------
 * U02–U03  monome connect event
 * --------------------------------------------------------------------------*/
static void test_monome_connect(void) {
    event_t e;
    s32 expected_data;

    usb_sim_reset();
    usb_sim_monome_connect(1 /*grid*/, 16 /*cols*/, 8 /*rows*/);

    ASSERT_EQ(event_queue_count(), 1, "U02: monome_connect posts one event");

    event_next(&e);
    ASSERT_EQ((int)e.type, (int)kEventMonomeConnect, "U02b: event type is kEventMonomeConnect");

    /* Expected packing: device=1, cols=16, rows=8 → 1 | (16<<8) | (8<<16) */
    expected_data = (s32)( (u32)1 | ((u32)16 << 8) | ((u32)8 << 16) );
    ASSERT_EQ((int)e.data, (int)expected_data,
              "U03: monome_connect data encodes device/cols/rows");
}

/* -------------------------------------------------------------------------
 * U04  monome disconnect
 * --------------------------------------------------------------------------*/
static void test_monome_disconnect(void) {
    event_t e;
    usb_sim_reset();
    usb_sim_monome_disconnect();
    ASSERT_EQ(event_queue_count(), 1, "U04a: monome_disconnect posts one event");
    event_next(&e);
    ASSERT_EQ((int)e.type, (int)kEventMonomeDisconnect, "U04b: type is kEventMonomeDisconnect");
    ASSERT_EQ((int)e.data, 0, "U04c: monome_disconnect data=0");
}

/* -------------------------------------------------------------------------
 * U05–U06  MIDI connect / disconnect
 * --------------------------------------------------------------------------*/
static void test_midi_lifecycle(void) {
    event_t e;

    usb_sim_reset();
    usb_sim_midi_connect();
    ASSERT_EQ(event_queue_count(), 1, "U05a: midi_connect posts one event");
    event_next(&e);
    ASSERT_EQ((int)e.type, (int)kEventMidiConnect, "U05b: type is kEventMidiConnect");

    usb_sim_reset();
    usb_sim_midi_disconnect();
    ASSERT_EQ(event_queue_count(), 1, "U06a: midi_disconnect posts one event");
    event_next(&e);
    ASSERT_EQ((int)e.type, (int)kEventMidiDisconnect, "U06b: type is kEventMidiDisconnect");
}

/* -------------------------------------------------------------------------
 * U07  MIDI packet injection
 * --------------------------------------------------------------------------*/
static void test_midi_packet_event(void) {
    event_t e;
    s32 expected;

    usb_sim_reset();
    usb_sim_midi_packet(0x09, 0x90, 0x3C, 0x64);  /* Note On, middle C, vel 100 */
    ASSERT_EQ(event_queue_count(), 1, "U07a: midi_packet posts one event");
    event_next(&e);
    ASSERT_EQ((int)e.type, (int)kEventMidiPacket, "U07b: type is kEventMidiPacket");
    expected = (s32)( (u32)0x09 | ((u32)0x90 << 8) | ((u32)0x3C << 16) | ((u32)0x64 << 24) );
    ASSERT_EQ((int)e.data, (int)expected, "U07c: midi_packet data encoded correctly");
}

/* -------------------------------------------------------------------------
 * U08  LED buffer all-zero after reset
 * --------------------------------------------------------------------------*/
static void test_led_buffer_zero_after_reset(void) {
    u8 buf[256];
    u16 len = 0;
    u16 i;
    int all_zero = 1;

    usb_sim_reset();
    usb_sim_get_led_buffer(buf, sizeof(buf), &len);
    for (i = 0; i < len; i++) {
        if (buf[i] != 0) { all_zero = 0; break; }
    }
    ASSERT_EQ(all_zero, 1, "U08: LED buffer all zero after reset");
}

/* -------------------------------------------------------------------------
 * U09  monome_led_set visible via get_led_buffer
 * --------------------------------------------------------------------------*/
static void test_led_set_visible(void) {
    u8 buf[256];
    u16 len = 0;

    usb_sim_reset();
    monome_led_set(3, 2, 7);  /* x=3, y=2, brightness=7 */
    usb_sim_get_led_buffer(buf, sizeof(buf), &len);
    /* Row-major: index = y * GRID_COLS + x = 2*16 + 3 = 35 */
    ASSERT_EQ((int)buf[35], 7, "U09: monome_led_set(3,2,7) visible in buffer at index 35");
}

/* -------------------------------------------------------------------------
 * U10  usb_sim_get_led_xy single-cell
 * --------------------------------------------------------------------------*/
static void test_get_led_xy(void) {
    usb_sim_reset();
    monome_led_set(5, 1, 12);
    ASSERT_EQ((int)usb_sim_get_led_xy(5, 1), 12, "U10: get_led_xy(5,1) returns 12");
    ASSERT_EQ((int)usb_sim_get_led_xy(0, 0),  0, "U10b: unset cell (0,0) returns 0");
    ASSERT_EQ((int)usb_sim_get_led_xy(99, 99), 0xff, "U10c: out-of-range returns 0xff");
}

/* -------------------------------------------------------------------------
 * U11  monome_led_all fills entire buffer
 * --------------------------------------------------------------------------*/
static void test_led_all(void) {
    u8 buf[256];
    u16 len = 0, i;
    int all_five = 1;

    usb_sim_reset();
    monome_led_all(5);
    usb_sim_get_led_buffer(buf, sizeof(buf), &len);
    for (i = 0; i < len; i++) {
        if (buf[i] != 5) { all_five = 0; break; }
    }
    ASSERT_EQ(all_five, 1, "U11: monome_led_all(5) fills entire buffer with 5");
}

/* -------------------------------------------------------------------------
 * U12  monome_led_map sets quadrant correctly
 * --------------------------------------------------------------------------*/
static void test_led_map(void) {
    /* Set top-left 8×8 quadrant: first row all on, rest off */
    u8 map[8] = { 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    usb_sim_reset();
    monome_led_map(0, 0, map);
    /* Row 0, col 0–7 should be 15 (from bit=1); col 8–15 unchanged (0) */
    ASSERT_EQ((int)usb_sim_get_led_xy(0, 0), 15, "U12a: map row0 col0 = 15");
    ASSERT_EQ((int)usb_sim_get_led_xy(7, 0), 15, "U12b: map row0 col7 = 15");
    ASSERT_EQ((int)usb_sim_get_led_xy(0, 1),  0, "U12c: map row1 = 0");
    ASSERT_EQ((int)usb_sim_get_led_xy(8, 0),  0, "U12d: outside quadrant = 0");
}

/* -------------------------------------------------------------------------
 * U13  LED buffer reset between test cases
 * --------------------------------------------------------------------------*/
static void test_led_reset(void) {
    monome_led_all(15);
    usb_sim_reset();
    ASSERT_EQ((int)usb_sim_get_led_xy(0, 0), 0,
              "U13: LED(0,0) = 0 after usb_sim_reset");
}

/* -------------------------------------------------------------------------
 * U14  MIDI out buffer empty after reset
 * --------------------------------------------------------------------------*/
static void test_midi_out_empty_after_reset(void) {
    usb_sim_reset();
    ASSERT_EQ((int)midiOutPacketCount, 0, "U14: MIDI out packet count = 0 after reset");
}

/* -------------------------------------------------------------------------
 * U15  uhi_midi_write captured in midi out buffer
 * --------------------------------------------------------------------------*/
static void test_midi_out_capture(void) {
    usb_sim_midi_packet_t pkts[8];
    u16 count = 0;

    usb_sim_reset();
    uhi_midi_write(0x09, 0x90, 0x3C, 0x64);  /* Note On */

    usb_sim_get_midi_out_buffer(pkts, 8, &count);
    ASSERT_EQ((int)count, 1, "U15a: one packet captured by uhi_midi_write");
    ASSERT_EQ((int)pkts[0].b[0], 0x09, "U15b: packet b0 = 0x09");
    ASSERT_EQ((int)pkts[0].b[1], 0x90, "U15c: packet b1 = 0x90 (Note On ch1)");
    ASSERT_EQ((int)pkts[0].b[2], 0x3C, "U15d: packet b2 = 0x3C (middle C)");
    ASSERT_EQ((int)pkts[0].b[3], 0x64, "U15e: packet b3 = 0x64 (vel 100)");
}

/* -------------------------------------------------------------------------
 * U16  multiple MIDI packets in order
 * --------------------------------------------------------------------------*/
static void test_midi_out_order(void) {
    usb_sim_midi_packet_t pkts[8];
    u16 count = 0;

    usb_sim_reset();
    uhi_midi_write(0x08, 0x80, 0x3C, 0x00);  /* Note Off */
    uhi_midi_write(0x0B, 0xB0, 0x07, 0x40);  /* CC 7 vol=64 */
    uhi_midi_write(0x09, 0x90, 0x48, 0x7F);  /* Note On E4 */

    usb_sim_get_midi_out_buffer(pkts, 8, &count);
    ASSERT_EQ((int)count, 3, "U16a: three packets captured");
    ASSERT_EQ((int)pkts[0].b[1], 0x80, "U16b: first packet is Note Off");
    ASSERT_EQ((int)pkts[1].b[1], 0xB0, "U16c: second packet is CC");
    ASSERT_EQ((int)pkts[2].b[1], 0x90, "U16d: third packet is Note On");
}

/* -------------------------------------------------------------------------
 * U17  MIDI buffer reset
 * --------------------------------------------------------------------------*/
static void test_midi_out_reset(void) {
    uhi_midi_write(0x09, 0x90, 0x3C, 0x64);
    usb_sim_reset();
    ASSERT_EQ((int)midiOutPacketCount, 0, "U17: MIDI out count = 0 after usb_sim_reset");
}

/* -------------------------------------------------------------------------
 * U18  ftdi_write captured in ftdiOutBuffer
 * --------------------------------------------------------------------------*/
static void test_ftdi_capture(void) {
    u8 frame[] = { 0x6A, 0x00, 0xFF, 0xAA };

    usb_sim_reset();
    ftdi_write(frame, 4);
    ASSERT_EQ((int)ftdiOutBufferLen, 4, "U18a: ftdi_write: 4 bytes captured");
    ASSERT_EQ((int)ftdiOutBuffer[0], 0x6A, "U18b: first byte correct");
    ASSERT_EQ((int)ftdiOutBuffer[2], 0xFF, "U18c: third byte correct");
}

/* -------------------------------------------------------------------------
 * U19  ftdi_out_buf_clear
 * --------------------------------------------------------------------------*/
static void test_ftdi_clear(void) {
    u8 frame[] = { 0x01, 0x02 };
    ftdi_write(frame, 2);
    ftdi_out_buf_clear();
    ASSERT_EQ((int)ftdiOutBufferLen, 0, "U19: ftdi_out_buf_clear sets len=0");
    ASSERT_EQ((int)ftdiOutBuffer[0],  0, "U19b: ftdi_out_buf_clear zeroes buffer");
}

/* -------------------------------------------------------------------------
 * U20  usb_sim_reset clears FTDI + MIDI atomically
 * --------------------------------------------------------------------------*/
static void test_reset_clears_all(void) {
    u8 frame[] = { 0xDE, 0xAD };
    ftdi_write(frame, 2);
    uhi_midi_write(0x09, 0x90, 0x3C, 0x64);
    monome_led_all(15);
    usb_sim_reset();
    ASSERT_EQ((int)ftdiOutBufferLen,     0, "U20a: FTDI buffer cleared by usb_sim_reset");
    ASSERT_EQ((int)midiOutPacketCount,   0, "U20b: MIDI buffer cleared by usb_sim_reset");
    ASSERT_EQ((int)usb_sim_get_led_xy(0,0), 0, "U20c: LED buffer cleared by usb_sim_reset");
    ASSERT_EQ(event_queue_count(),       0, "U20d: event queue cleared by usb_sim_reset");
}

/* -------------------------------------------------------------------------
 * U21  CDC transport distinct from FTDI
 * --------------------------------------------------------------------------*/
static void test_cdc_transport_distinct(void) {
    usb_sim_reset();
    
    /* Connect a CDC grid */
    usb_sim_monome_connect_cdc(1, 16, 8);
    ASSERT_EQ((int)usb_sim_get_transport(), (int)eUsbSimTransportCDC,
              "U21a: CDC connect sets transport to CDC");
    ASSERT_EQ((int)monome_transport_get(), (int)eTransportCDC,
              "U21b: monome_transport reflects CDC");
    
    /* Write should go to CDC buffer, not FTDI */
    u8 frame[] = { 0x1A, 0x00, 0x00, 0xFF };
    monome_transport_write(frame, 4);
    ASSERT_EQ((int)cdcOutBufferLen, 4, "U21c: CDC write captured in cdcOutBuffer");
    ASSERT_EQ((int)ftdiOutBufferLen, 0, "U21d: FTDI buffer untouched by CDC write");
    
    usb_sim_monome_disconnect();
    ASSERT_EQ((int)usb_sim_get_transport(), (int)eUsbSimTransportNone,
              "U21e: disconnect resets transport to None");
}

/* -------------------------------------------------------------------------
 * U22  FTDI transport still works after CDC addition
 * --------------------------------------------------------------------------*/
static void test_ftdi_transport_still_works(void) {
    usb_sim_reset();
    
    /* Connect an FTDI grid */
    usb_sim_monome_connect_ftdi(1, 16, 8);
    ASSERT_EQ((int)usb_sim_get_transport(), (int)eUsbSimTransportFTDI,
              "U22a: FTDI connect sets transport to FTDI");
    ASSERT_EQ((int)monome_transport_get(), (int)eTransportFTDI,
              "U22b: monome_transport reflects FTDI");
    
    /* Write should go to FTDI buffer, not CDC */
    u8 frame[] = { 0x1A, 0x00, 0x00, 0xFF };
    monome_transport_write(frame, 4);
    ASSERT_EQ((int)ftdiOutBufferLen, 4, "U22c: FTDI write captured in ftdiOutBuffer");
    ASSERT_EQ((int)cdcOutBufferLen, 0, "U22d: CDC buffer untouched by FTDI write");
    
    usb_sim_monome_disconnect();
}

/* -------------------------------------------------------------------------
 * U23  Legacy usb_sim_monome_connect defaults to FTDI
 * --------------------------------------------------------------------------*/
static void test_legacy_connect_defaults_ftdi(void) {
    usb_sim_reset();
    
    /* Legacy API should default to FTDI for backward compatibility */
    usb_sim_monome_connect(1, 16, 8);
    ASSERT_EQ((int)usb_sim_get_transport(), (int)eUsbSimTransportFTDI,
              "U23: legacy usb_sim_monome_connect defaults to FTDI");
    
    usb_sim_monome_disconnect();
}

/* -------------------------------------------------------------------------
 * U24  usb_sim_reset clears both FTDI and CDC buffers
 * --------------------------------------------------------------------------*/
static void test_reset_clears_both_buffers(void) {
    u8 frame[] = { 0xDE, 0xAD };
    
    /* Fill FTDI buffer */
    ftdi_write(frame, 2);
    /* Fill CDC buffer */
    cdc_write(frame, 2);
    
    usb_sim_reset();
    ASSERT_EQ((int)ftdiOutBufferLen, 0, "U24a: FTDI buffer cleared");
    ASSERT_EQ((int)cdcOutBufferLen, 0, "U24b: CDC buffer cleared");
    ASSERT_EQ((int)usb_sim_get_transport(), (int)eUsbSimTransportNone,
              "U24c: transport reset to None");
}

/* -------------------------------------------------------------------------
 * Main
 * --------------------------------------------------------------------------*/
int run_usb_sim_tests(void) {
    printf("TAP version 13\n");
    printf("# USB Simulation Layer Test Suite\n");
    printf("1..50\n");   /* upper bound; real count printed at end */

    test_reset_clears_queue();
    test_monome_connect();
    test_monome_disconnect();
    test_midi_lifecycle();
    test_midi_packet_event();
    test_led_buffer_zero_after_reset();
    test_led_set_visible();
    test_get_led_xy();
    test_led_all();
    test_led_map();
    test_led_reset();
    test_midi_out_empty_after_reset();
    test_midi_out_capture();
    test_midi_out_order();
    test_midi_out_reset();
    test_ftdi_capture();
    test_ftdi_clear();
    test_reset_clears_all();
    test_cdc_transport_distinct();
    test_ftdi_transport_still_works();
    test_legacy_connect_defaults_ftdi();
    test_reset_clears_both_buffers();

    printf("\n# Ran %d assertions, %d failed\n", _test_count, _fail_count);
    if (_fail_count == 0) {
        printf("# RESULT: PASS\n");
    } else {
        printf("# RESULT: FAIL\n");
    }
    return _fail_count > 0 ? 1 : 0;
}
