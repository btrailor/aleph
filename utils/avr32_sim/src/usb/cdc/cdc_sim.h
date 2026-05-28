/* cdc_sim.h
   aleph / avr32_sim / usb/cdc

   CDC (Communication Device Class) simulation header for modern monome grids.

   Provides the same functional interface as ftdi.h so that monome.c can use
   either transport interchangeably via the monome_transport abstraction.

   This is the simulation counterpart to avr32/src/usb/cdc/cdc.h,
   adapted for the host-based simulator (no actual USB stack).
*/

#ifndef _ALEPH_CDC_SIM_H_
#define _ALEPH_CDC_SIM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

//------ defines
#define CDC_RX_BUF_SIZE 64
#define CDC_OUT_BUF_SIZE 4096

//------ extern function declarations

// initialization
extern void cdc_init(void);

// device connection/disconnection
extern void cdc_setup(void);
extern void cdc_disconnect(void);

// data transmission
extern int cdc_write(const u8* data, u16 bytes);
extern volatile u8 cdc_tx_busy(void);

// data reception
extern void cdc_read(void);
extern u8* cdc_rx_buf(void);
extern volatile u8 cdc_rx_bytes(void);
extern volatile u8 cdc_rx_busy(void);

// device status
extern u8 cdc_connected(void);

// capture buffer (simulation-specific)
extern u8  cdcOutBuffer[CDC_OUT_BUF_SIZE];
extern u16 cdcOutBufferLen;
extern void cdc_out_buf_clear(void);

#ifdef __cplusplus
}
#endif

#endif // h guard
