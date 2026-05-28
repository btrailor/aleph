/* sdl_screen.c
 * aleph / avr32_sim
 *
 * SDL2 Screen Renderer — Phase 3 implementation.
 *
 * Creates a 512×256 window (128×64 OLED scaled 4×).
 * Uses SDL2 software renderer for maximum compatibility.
 */

#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "sdl_screen.h"

static SDL_Window   *window   = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture  *texture  = NULL;
static int           running  = 0;

// Framebuffer used by the sim (128×64, 1 byte per pixel for simplicity)
static u8 sim_framebuffer[SDL_SCREEN_W * SDL_SCREEN_H];

// SIM: Track FPS timing
static Uint32 frameStartMs = 0;
#define TARGET_FPS      30
#define FRAME_MS        (1000 / TARGET_FPS)

int sdl_screen_init(void) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "[sdl_screen] SDL_Init failed: %s\n", SDL_GetError());
    return -1;
  }

  window = SDL_CreateWindow(
    "Aleph Simulator",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    SDL_WINDOW_W, SDL_WINDOW_H,
    SDL_WINDOW_SHOWN
  );
  if (!window) {
    fprintf(stderr, "[sdl_screen] SDL_CreateWindow failed: %s\n", SDL_GetError());
    SDL_Quit();
    return -1;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  if (!renderer) {
    fprintf(stderr, "[sdl_screen] SDL_CreateRenderer failed: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
  }

  // Create streaming texture for pixel data
  texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGB888,
    SDL_TEXTUREACCESS_STREAMING,
    SDL_SCREEN_W, SDL_SCREEN_H
  );
  if (!texture) {
    fprintf(stderr, "[sdl_screen] SDL_CreateTexture failed: %s\n", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
  }

  // Clear framebuffer
  memset(sim_framebuffer, 0, sizeof(sim_framebuffer));

  running = 1;
  frameStartMs = SDL_GetTicks();
  fprintf(stderr, "[sdl_screen] initialized %dx%d window (OLED %dx%d @ %dx scale)\n",
          SDL_WINDOW_W, SDL_WINDOW_H, SDL_SCREEN_W, SDL_SCREEN_H, SDL_SCREEN_SCALE);
  return 0;
}

void sdl_screen_quit(void) {
  running = 0;
  if (texture)  { SDL_DestroyTexture(texture);  texture = NULL; }
  if (renderer) { SDL_DestroyRenderer(renderer); renderer = NULL; }
  if (window)   { SDL_DestroyWindow(window);     window = NULL; }
  SDL_Quit();
  fprintf(stderr, "[sdl_screen] quit\n");
}

int sdl_screen_running(void) {
  return running;
}

void sdl_screen_blit(const u8 *framebuffer) {
  int pitch;
  void *pixels;
  int x, y;
  u8 *dst;
  u8 pix;

  if (!texture || !framebuffer) return;

  // Lock texture for writing
  if (SDL_LockTexture(texture, NULL, &pixels, &pitch) != 0) {
    return;
  }

  // Copy monochrome framebuffer to RGB texture
  // OLED: 0 = off (black), non-zero = on (white)
  for (y = 0; y < SDL_SCREEN_H; y++) {
    dst = (u8 *)pixels + (y * pitch);
    for (x = 0; x < SDL_SCREEN_W; x++) {
      pix = framebuffer[y * SDL_SCREEN_W + x];
      // RGB888: white if pixel on, black if off
      dst[x * 3 + 0] = pix ? 0xff : 0x00;  // R
      dst[x * 3 + 1] = pix ? 0xff : 0x00;  // G
      dst[x * 3 + 2] = pix ? 0xff : 0x00;  // B
    }
  }

  SDL_UnlockTexture(texture);
}

void sdl_screen_present(void) {
  SDL_Rect dstRect;

  if (!renderer || !texture) return;

  // Render texture scaled to full window
  dstRect.x = 0;
  dstRect.y = 0;
  dstRect.w = SDL_WINDOW_W;
  dstRect.h = SDL_WINDOW_H;

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, &dstRect);
  SDL_RenderPresent(renderer);
}

void sdl_screen_frame_delay(void) {
  Uint32 now = SDL_GetTicks();
  Uint32 elapsed = now - frameStartMs;
  if (elapsed < FRAME_MS) {
    SDL_Delay(FRAME_MS - elapsed);
  }
  frameStartMs = SDL_GetTicks();
}
