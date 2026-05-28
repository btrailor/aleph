/* cdc_sim.c
 * aleph / avr32_sim / usb/cdc
 *
 * CDC (Communication Device Class) simulation stub for modern monome grids.
 *
 * Real behaviour: CDC grids use USB CDC-ACM protocol (virtual serial port)
 * instead of FTDI USB-serial. The protocol is identical at the serialosc
 * level, but the USB transport layer differs.
 *
 * Sim behaviour: mirrors ftdi.c structure but with CDC naming. Provides
 * capture buffer for test verification and connects to the transport
 * abstraction layer.
 *
 * This enables the emulator to distinguish between FTDI-based legacy grids
 * and CDC-based modern grids (e.g., monome one, grid 2022+).
 */

#include <string.h>
#include <stdio.h>

#include "cdc_sim.h"

/* -------------------------------------------------------------------------
 * Capture buffer (mirrors ftdiOutBuffer for test compatibility)
 * --------------------------------------------------------------------------*/
u8  cdcOutBuffer[CDC_OUT_BUF_SIZE];
u16 cdcOutBufferLen = 0;

/* -------------------------------------------------------------------------
 * cdc_write
 * --------------------------------------------------------------------------*/
int cdc_write(const u8 *buf, u16 len) {
    u16 i;
    int accepted = 0;

    if (buf == NULL || len == 0) { return 0; }

    for (i = 0; i < len; i++) {
        if (cdcOutBufferLen >= CDC_OUT_BUF_SIZE) {
            fprintf(stderr,
                    "[cdc] output buffer full (%u bytes), dropping %u bytes\n",
                    (unsigned)CDC_OUT_BUF_SIZE,
                    (unsigned)(len - i));
            break;
        }
        cdcOutBuffer[cdcOutBufferLen++] = buf[i];
        accepted++;
    }

    return accepted;
}

/* -------------------------------------------------------------------------
 * cdc_out_buf_clear
 * --------------------------------------------------------------------------*/
void cdc_out_buf_clear(void) {
    memset(cdcOutBuffer, 0, sizeof(cdcOutBuffer));
    cdcOutBufferLen = 0;
}

/* -------------------------------------------------------------------------
 * Connection state
 * --------------------------------------------------------------------------*/
static u8 cdc_connected_flag = 0;

void cdc_setup(void) {
    cdc_connected_flag = 1;
    cdc_out_buf_clear();
}

void cdc_disconnect(void) {
    cdc_connected_flag = 0;
}

u8 cdc_connected(void) {
    return cdc_connected_flag;
}

/* -------------------------------------------------------------------------
 * Stubs for functions declared in cdc_sim.h but not implemented in sim
 * --------------------------------------------------------------------------*/

void cdc_read(void) { }

u8* cdc_rx_buf(void) { return NULL; }

volatile u8 cdc_rx_bytes(void) { return 0; }

volatile u8 cdc_rx_busy(void) { return 0; }

volatile u8 cdc_tx_busy(void) { return 0; }

/* -------------------------------------------------------------------------
 * cdc_init
 * --------------------------------------------------------------------------*/
void cdc_init(void) {
    cdc_connected_flag = 0;
    cdc_out_buf_clear();
}
