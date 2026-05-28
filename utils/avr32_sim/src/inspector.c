/* inspector.c
 * aleph / avr32_sim
 *
 * Operator Parameter Inspector — Phase 3 implementation.
 *
 * Semi-transparent overlay in top-right corner of SDL window.
 * Shows current scene name, selected operator, and param values.
 */

#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "inspector.h"
#include "sdl_screen.h"

// External SDL renderer (from sdl_screen.c)
extern SDL_Renderer *renderer;

static int visible = 0;
static int selected_op = 0;
static int selected_param = 0;

// SIM: stub scene name
static char scene_name[32] = "untitled";

void inspector_init(void) {
  visible = 0;
  selected_op = 0;
  selected_param = 0;
  strncpy(scene_name, "untitled", sizeof(scene_name) - 1);
  scene_name[sizeof(scene_name) - 1] = '\0';
}

void inspector_toggle(void) {
  visible = !visible;
  fprintf(stderr, "[inspector] %s\n", visible ? "shown" : "hidden");
}

int inspector_visible(void) {
  return visible;
}

void inspector_draw(void *sdl_renderer) {
  SDL_Renderer *r = (SDL_Renderer *)sdl_renderer;
  SDL_Rect bg;
  char buf[64];

  if (!visible || !r) return;

  // Background: semi-transparent black box, top-right
  bg.x = SDL_WINDOW_W - 200;
  bg.y = 10;
  bg.w = 190;
  bg.h = 120;
  SDL_SetRenderDrawColor(r, 0, 0, 0, 200);
  SDL_RenderFillRect(r, &bg);

  // Border
  SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
  SDL_RenderDrawRect(r, &bg);

  // SIM: Text rendering would require SDL_ttf. For now we just draw
  // the border to show the inspector is active. In a full implementation
  // we'd render text with SDL_ttf or a built-in bitmap font.
  (void)snprintf(buf, sizeof(buf), "op:%d par:%d", selected_op, selected_param);
  // TODO: render text
  (void)buf;
}

void inspector_next_op(void) {
  selected_op++;
  if (selected_op > 15) selected_op = 0;
}

void inspector_prev_op(void) {
  selected_op--;
  if (selected_op < 0) selected_op = 15;
}

void inspector_param_inc(void) {
  selected_param++;
}

void inspector_param_dec(void) {
  selected_param--;
}

int inspector_handle_key(int sdl_key) {
  if (!visible) return 0;

  switch (sdl_key) {
    case SDLK_TAB:
      if (SDL_GetModState() & KMOD_SHIFT) {
        inspector_prev_op();
      } else {
        inspector_next_op();
      }
      return 1;
    case SDLK_PLUS:
    case SDLK_EQUALS:
      inspector_param_inc();
      return 1;
    case SDLK_MINUS:
      inspector_param_dec();
      return 1;
    default:
      return 0;
  }
}
