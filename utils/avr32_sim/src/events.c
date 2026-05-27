/* events.c
 * aleph
 *
 * simple event queue
 *
 * SIM CHANGES (Phase 0):
 *   - Uncommented queue state (putIdx, getIdx, sysEvents[])
 *   - Removed cpu_irq_disable_level / cpu_irq_enable_level calls; those are
 *     AVR32-specific IRQ primitives that don't exist on host. On the simulator
 *     we run single-threaded so no guard is needed.
 *   - init_events now actually zeroes the queue.
 */

#include "print_funcs.h"
#include "events.h"
#include "event_types.h"

/// NOTE: if we are ever over-filling the event queue, we have problems.
/// making the event queue bigger not likely to solve the problems.
#define MAX_EVENTS   32

// macro for incrementing an index into a circular buffer.
#define INCR_EVENT_INDEX( x )  { if ( ++x == MAX_EVENTS ) x = 0; }

// get/put indexes into sysEvents[] array
static int putIdx = 0;
static int getIdx = 0;

// The system event queue is a circular array of event records.
static event_t sysEvents[ MAX_EVENTS ];

// initializes (or re-initializes) the system event queue.
void init_events( void ) {
  int k;

  putIdx = 0;
  getIdx = 0;

  for ( k = 0; k < MAX_EVENTS; k++ ) {
    sysEvents[ k ].type = 0;
    sysEvents[ k ].data = 0;
  }
}

// get next event
// Returns non-zero if an event was available
u8 event_next( event_t *e ) {
  u8 status;

  // SIM: no IRQ guard needed (single-threaded host)
  // if pointers are equal, the queue is empty
  if ( getIdx != putIdx ) {
    INCR_EVENT_INDEX( getIdx );
    e->type = sysEvents[ getIdx ].type;
    e->data = sysEvents[ getIdx ].data;
    status = 1;
  } else {
    e->type  = 0xff;
    e->data = 0;
    status = 0;
  }

  return status;
}

// add event to queue, return success status
u8 event_post( event_t *e ) {
  u8 status = 0;
  int saveIndex;

  // SIM: no IRQ guard needed (single-threaded host)
  saveIndex = putIdx;
  INCR_EVENT_INDEX( putIdx );
  if ( putIdx != getIdx ) {
    sysEvents[ putIdx ].type = e->type;
    sysEvents[ putIdx ].data = e->data;
    status = 1;
  } else {
    // idx wrapped, queue is full — restore idx
    putIdx = saveIndex;
    print_dbg("\r\n event queue full!");
  }

  return status;
}
