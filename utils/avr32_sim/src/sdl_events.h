/* sdl_events.h
 * aleph / avr32_sim
 *
 * SDL2 Event Loop Bridging — Phase 3
 *
 * Polls SDL keyboard/mouse events and injects them into the
 * simulator's event queue via event_post().
 */

#ifndef _SDL_EVENTS_H_
#define _SDL_EVENTS_H_

#include "types.h"

// Encoder numbers (matching hardware)
#define SDL_ENC_0   0   // Left encoder
#define SDL_ENC_1   1   // Right encoder

// Switch numbers (matching hardware)
#define SDL_SW_0    0   // SW0 (mode)
#define SDL_SW_1    1   // SW1 (edit)

// Process pending SDL events. Returns 0 if quit requested, 1 otherwise.
int sdl_events_poll(void);

// Map an SDL keycode to simulator action and post event.
// Returns 1 if key was handled, 0 if unmapped.
int sdl_events_map_key(int sdl_key);

// Map SDL mouse position to monome grid press (if grid connected).
// x, y are pixel coordinates in the 512×256 window.
void sdl_events_map_mouse(int x, int y, int pressed);

#endif // _SDL_EVENTS_H_
