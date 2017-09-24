#pragma once

#include <bb.h>
#include <SDL2/SDL.h>

#include "input.h"

struct Window {
  uint32_t width;
  uint32_t height;
  bool fullscreen;
  const char* title;
  bool is_close_requested;

  Input input;

  SDL_Window* sdl_window;
  SDL_GLContext gl_context;
};

Window* openWindow(uint32_t width, uint32_t height, const char* title, bool fullscreen);
void closeWindow(Window* window);
void catchEvents(Window* window);
void swapBuffers(Window* window);

