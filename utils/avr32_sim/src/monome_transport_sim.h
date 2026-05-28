/* monome_transport_sim.h
   aleph / avr32_sim

   Transport abstraction layer for monome communication in the simulator.
   Mirrors the real firmware's monome_transport.h but uses simulator-specific
   FTDI and CDC stubs.

   This enables the emulator to treat FTDI-based legacy grids and CDC-based
   modern grids distinctly.
*/

#ifndef _ALEPH_MONOME_TRANSPORT_SIM_H_
#define _ALEPH_MONOME_TRANSPORT_SIM_H_

#include "types.h"

//------ defines
#define MONOME_TRANSPORT_TX_BUF_LEN 72

//------ typedefs

// transport type enumeration
typedef enum {
  eTransportNone,     // no transport selected
  eTransportFTDI,     // FTDI-based transport (older grids)
  eTransportCDC,      // CDC-based transport (modern grids)
  eTransportNumTransports // count
} eMonomeTransport;

// transport function pointers (matching volatile return types in ftdi.h / cdc_sim.h)
typedef int (*transport_write_t)(const u8* data, u16 bytes);
typedef volatile u8 (*transport_tx_busy_t)(void);
typedef void (*transport_read_t)(void);
typedef volatile u8 (*transport_rx_busy_t)(void);
typedef volatile u8 (*transport_rx_bytes_t)(void);
typedef u8* (*transport_rx_buf_t)(void);
typedef void (*transport_setup_t)(void);
typedef void (*transport_disconnect_t)(void);
typedef u8 (*transport_connected_t)(void);

//------ extern variables

// current transport type
extern eMonomeTransport monome_transport_type;

//------ extern function declarations

// transport management
extern void monome_transport_init(void);
extern void monome_transport_set(eMonomeTransport transport);
extern eMonomeTransport monome_transport_get(void);

// unified transport interface
extern int monome_transport_write(const u8* data, u16 bytes);
extern volatile u8 monome_transport_tx_busy(void);
extern void monome_transport_read(void);
extern u8 monome_transport_rx_busy(void);
extern u8 monome_transport_rx_bytes(void);
extern u8* monome_transport_rx_buf(void);
extern void monome_transport_setup(void);
extern void monome_transport_disconnect(void);
extern u8 monome_transport_connected(void);

// transport-specific setup functions
extern void monome_transport_setup_ftdi(void);
extern void monome_transport_setup_cdc(void);

#endif // h guard
