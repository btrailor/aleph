/* sdl_screen.h
 * aleph / avr32_sim
 *
 * SDL2 Screen Renderer — Phase 3
 *
 * Renders the Aleph's 128×64 monochrome OLED via SDL2,
 * scaling 4× for visibility on modern displays.
 */

#ifndef _SDL_SCREEN_H_
#define _SDL_SCREEN_H_

#include "types.h"

// OLED dimensions
#define SDL_SCREEN_W        128
#define SDL_SCREEN_H        64
#define SDL_SCREEN_SCALE    4
#define SDL_WINDOW_W        (SDL_SCREEN_W * SDL_SCREEN_SCALE)
#define SDL_WINDOW_H        (SDL_SCREEN_H * SDL_SCREEN_SCALE)

// Pixel values in framebuffer (matching OLED: 0=off, non-zero=on)
#define SDL_PIX_OFF         0
#define SDL_PIX_ON          0xff

// Opaque handle
struct sdl_screen;
typedef struct sdl_screen sdl_screen_t;

// Initialize SDL window and renderer. Returns 0 on success, -1 on error.
int sdl_screen_init(void);

// Shutdown SDL.
void sdl_screen_quit(void);

// Returns 1 while window is open, 0 after quit requested.
int sdl_screen_running(void);

// Blit a 128×64 framebuffer (u8 array, 0=off, non-zero=on) to the SDL window.
// Call this each frame after the sim has rendered to its framebuffer.
void sdl_screen_blit(const u8 *framebuffer);

// Present the current frame to the window.
void sdl_screen_present(void);

// Delay to maintain target FPS (30 FPS default).
void sdl_screen_frame_delay(void);

#endif // _SDL_SCREEN_H_
