/* sdl_events.c
 * aleph / avr32_sim
 *
 * SDL2 Event Loop Bridging — Phase 3 implementation.
 *
 * Maps keyboard input to Aleph encoders/switches and injects
 * events into the simulator's event queue.
 */

#include <SDL.h>
#include <stdio.h>

#include "sdl_events.h"
#include "sdl_screen.h"
#include "events.h"
#include "event_types.h"
#include "event_inject.h"

// External: declared in sdl_screen.c
extern int sdl_screen_running(void);
extern void sdl_screen_quit(void);

// Current encoder deltas (accumulated between polls)
static s8 enc_delta[2] = {0, 0};

// SIM: Track switch state for press/release
static u8 sw_pressed[2] = {0, 0};

int sdl_events_poll(void) {
  SDL_Event e;

  while (SDL_PollEvent(&e)) {
    switch (e.type) {
      case SDL_QUIT:
        sdl_screen_quit();
        return 0;

      case SDL_KEYDOWN:
        if (e.key.keysym.sym == SDLK_ESCAPE) {
          sdl_screen_quit();
          return 0;
        }
        sdl_events_map_key((int)e.key.keysym.sym);
        break;

      case SDL_KEYUP:
        // Release switches on keyup
        if (e.key.keysym.sym == SDLK_SPACE && sw_pressed[SDL_SW_0]) {
          sw_pressed[SDL_SW_0] = 0;
          inject_switch(SDL_SW_0, 0);
        }
        if (e.key.keysym.sym == SDLK_RETURN && sw_pressed[SDL_SW_1]) {
          sw_pressed[SDL_SW_1] = 0;
          inject_switch(SDL_SW_1, 0);
        }
        break;

      case SDL_MOUSEBUTTONDOWN:
        if (e.button.button == SDL_BUTTON_LEFT) {
          sdl_events_map_mouse(e.button.x, e.button.y, 1);
        }
        break;

      case SDL_MOUSEBUTTONUP:
        if (e.button.button == SDL_BUTTON_LEFT) {
          sdl_events_map_mouse(e.button.x, e.button.y, 0);
        }
        break;

      default:
        break;
    }
  }

  // Flush accumulated encoder deltas as events
  if (enc_delta[SDL_ENC_0] != 0) {
    inject_encoder(SDL_ENC_0, enc_delta[SDL_ENC_0]);
    enc_delta[SDL_ENC_0] = 0;
  }
  if (enc_delta[SDL_ENC_1] != 0) {
    inject_encoder(SDL_ENC_1, enc_delta[SDL_ENC_1]);
    enc_delta[SDL_ENC_1] = 0;
  }

  return 1;
}

int sdl_events_map_key(int sdl_key) {
  switch (sdl_key) {
    // Encoder 0: left/right arrows
    case SDLK_LEFT:
      enc_delta[SDL_ENC_0] -= 1;
      return 1;
    case SDLK_RIGHT:
      enc_delta[SDL_ENC_0] += 1;
      return 1;

    // Encoder 1: up/down arrows
    case SDLK_UP:
      enc_delta[SDL_ENC_1] -= 1;
      return 1;
    case SDLK_DOWN:
      enc_delta[SDL_ENC_1] += 1;
      return 1;

    // SW0 (mode): space
    case SDLK_SPACE:
      if (!sw_pressed[SDL_SW_0]) {
        sw_pressed[SDL_SW_0] = 1;
        inject_switch(SDL_SW_0, 1);
      }
      return 1;

    // SW1 (edit): return
    case SDLK_RETURN:
      if (!sw_pressed[SDL_SW_1]) {
        sw_pressed[SDL_SW_1] = 1;
        inject_switch(SDL_SW_1, 1);
      }
      return 1;

    default:
      return 0;
  }
}

void sdl_events_map_mouse(int x, int y, int pressed) {
  // Map window coordinates to monome grid coordinates
  // Window is 512×256, grid area is the full window (for now)
  // Scale: 512/16 = 32 pixels per grid cell, 256/8 = 32 pixels per grid cell
  int grid_x = x / 32;
  int grid_y = y / 32;

  if (grid_x < 0 || grid_x >= 16 || grid_y < 0 || grid_y >= 8) {
    return;
  }

  // SIM: inject monome press/release via event system
  // For now, just log. Full monome grid simulation would require
  // posting kEventMonomeGridKey events.
  fprintf(stderr, "[sdl_events] monome grid (%d,%d) %s\n",
          grid_x, grid_y, pressed ? "press" : "release");
}
