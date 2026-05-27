/* ftdi.c
 * aleph / avr32_sim / usb/ftdi
 *
 * FTDI USB-serial stub — outbound LED data logger.
 *
 * Real behaviour: writes serialosc-protocol bytes to the FTDI FIFO,
 * which the monome grid reads over USB.
 *
 * Sim behaviour: appends bytes to ftdiOutBuffer so test code can verify
 * that BEES produced the expected LED output without real hardware.
 *
 * Wire-up: if monome.c / BEES calls ftdi_write() to flush LED frames,
 * those bytes land here.  usb_sim_get_led_buffer() then exposes them
 * to the test layer.  For tests that work directly at the monome_led_*
 * level, monome_led_flush() / usb_sim_get_led_buffer() is the simpler
 * path — use whichever is appropriate.
 */

#include <string.h>
#include <stdio.h>

#include "ftdi.h"

/* -------------------------------------------------------------------------
 * Capture buffer
 * --------------------------------------------------------------------------*/
u8  ftdiOutBuffer[FTDI_OUT_BUF_SIZE];
u16 ftdiOutBufferLen = 0;

/* -------------------------------------------------------------------------
 * ftdi_write
 * --------------------------------------------------------------------------*/
int ftdi_write(const u8 *buf, u16 len) {
    u16 i;
    int accepted = 0;

    if (buf == NULL || len == 0) { return 0; }

    for (i = 0; i < len; i++) {
        if (ftdiOutBufferLen >= FTDI_OUT_BUF_SIZE) {
            /* Buffer full — log a warning and stop accepting. */
            fprintf(stderr,
                    "[ftdi] output buffer full (%u bytes), dropping %u bytes\n",
                    (unsigned)FTDI_OUT_BUF_SIZE,
                    (unsigned)(len - i));
            break;
        }
        ftdiOutBuffer[ftdiOutBufferLen++] = buf[i];
        accepted++;
    }

    return accepted;
}

/* -------------------------------------------------------------------------
 * ftdi_out_buf_clear
 * --------------------------------------------------------------------------*/
void ftdi_out_buf_clear(void) {
    memset(ftdiOutBuffer, 0, sizeof(ftdiOutBuffer));
    ftdiOutBufferLen = 0;
}

/* -------------------------------------------------------------------------
 * Stubs for functions declared in ftdi.h but not implemented
 * --------------------------------------------------------------------------*/

void ftdi_read(void) { }

u8* ftdi_rx_buf(void) { return NULL; }

volatile u8 ftdi_rx_bytes(void) { return 0; }

volatile u8 ftdi_rx_busy(void) { return 0; }

volatile u8 ftdi_tx_busy(void) { return 0; }

u8 ftdi_connected(void) { return 0; }

void ftdi_setup(void) { }
