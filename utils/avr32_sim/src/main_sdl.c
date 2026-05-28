/* main_sdl.c
 * aleph / avr32_sim
 *
 * SDL2 Main Application — Phase 3
 *
 * Entry point for the visual Aleph simulator.
 * Initializes SDL, runs the event loop, and bridges to the sim.
 */

#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "sdl_screen.h"
#include "sdl_events.h"
#include "inspector.h"
#include "events.h"
#include "monome.h"
#include "app_timers.h"

// SIM: workingDir required by files.c / ui_files.c
char workingDir[256] = "";

// SIM: The framebuffer that the sim renders to (128×64, 1 byte per pixel)
static u8 sdl_framebuffer[SDL_SCREEN_W * SDL_SCREEN_H];

// SIM: Forward declarations for sim tick
extern void process_events(void);
extern void update_screen(void);

// SIM: One frame of simulation
static void sim_tick(void) {
  // Process any pending sim events
  event_t e;
  while (event_next(&e)) {
    // Events are consumed; in a full implementation,
    // this would dispatch to the app event handlers.
    (void)e;
  }

  // Update screen rendering
  // In a full implementation, this would copy from the sim's
  // GRAM (graphics RAM) to sdl_framebuffer.
  // For now, leave framebuffer as-is.
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  fprintf(stderr, "[aleph-sdl] Aleph Simulator — Phase 3\n");

  // Initialize SDL screen
  if (sdl_screen_init() != 0) {
    fprintf(stderr, "[aleph-sdl] Failed to initialize SDL. Is SDL2 installed?\n");
    fprintf(stderr, "[aleph-sdl] Install with: brew install sdl2\n");
    return 1;
  }

  // Initialize simulator
  init_events();
  init_app_timers();
  init_monome();

  // Initialize inspector
  inspector_init();

  // Clear framebuffer
  memset(sdl_framebuffer, 0, sizeof(sdl_framebuffer));

  fprintf(stderr, "[aleph-sdl] Running. Keys: arrows=encoders, space=SW0, enter=SW1, I=inspector, ESC=quit\n");

  // Main loop
  while (sdl_screen_running()) {
    // Poll SDL events and inject into sim
    if (!sdl_events_poll()) {
      break;
    }

    // Advance simulation one frame
    sim_tick();

    // Render to SDL window
    sdl_screen_blit(sdl_framebuffer);
    inspector_draw(NULL);  // TODO: pass SDL renderer
    sdl_screen_present();

    // Frame rate limit
    sdl_screen_frame_delay();
  }

  fprintf(stderr, "[aleph-sdl] shutting down\n");
  sdl_screen_quit();
  return 0;
}
