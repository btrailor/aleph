/* inspector.h
 * aleph / avr32_sim
 *
 * Operator Parameter Inspector — Phase 3
 *
 * Overlay on SDL window showing current operator params.
 * Toggle with 'I' key.
 */

#ifndef _INSPECTOR_H_
#define _INSPECTOR_H_

#include "types.h"

// Initialize inspector state.
void inspector_init(void);

// Toggle visibility.
void inspector_toggle(void);

// Returns 1 if visible, 0 if hidden.
int inspector_visible(void);

// Draw inspector overlay using SDL renderer.
// Must be called between SDL_RenderClear() and SDL_RenderPresent().
void inspector_draw(void *sdl_renderer);

// Navigate to next/previous operator.
void inspector_next_op(void);
void inspector_prev_op(void);

// Increment/decrement current param.
void inspector_param_inc(void);
void inspector_param_dec(void);

// Handle key for inspector navigation.
// Returns 1 if key was consumed, 0 if not an inspector key.
int inspector_handle_key(int sdl_key);

#endif // _INSPECTOR_H_
