#include "window.h"
#include <GL/glew.h>

// imgui
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_sdl_gl3.h"

Window* openWindow(uint32_t width, uint32_t height, const char* title, bool fullscreen) {
  Window* window = (Window*)malloc(sizeof(Window));
 
  window->width = width;
  window->height = height;
  window->fullscreen = fullscreen;
  window->title = title;
  window->is_close_requested = false;

  SDL_Init(SDL_INIT_EVERYTHING);

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  uint32_t flags = SDL_WINDOW_OPENGL;// | SDL_WINDOW_RESIZABLE;

  if (fullscreen)
    flags |= SDL_WINDOW_FULLSCREEN;

  window->sdl_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);

  bb_assert(window->sdl_window != 0);

  window->gl_context = SDL_GL_CreateContext(window->sdl_window);

  // glew
  glewExperimental = GL_TRUE;
  GLenum result = glewInit();
  if (result != GLEW_OK) {
    printf("glew error: \"%s\"\n", glewGetErrorString(result)); 
    bb_assert(true);
  }

  // reset input:
   for (unsigned int i = 0; i < NUM_KEYS; i++) {
    window->input.keys_down[i] = false;
    window->input.keys_up[i] = false;
    window->input.keys[i] = false;
  }

  for (unsigned int i = 0; i < NUM_MOUSE_BUTTONS; i++) {
    window->input.buttons_down[i] = false;
    window->input.buttons_up[i] = false;
    window->input.buttons[i] = false;
  }

  return window;
}

void closeWindow(Window* window) {
  SDL_GL_DeleteContext(window->gl_context);
  SDL_DestroyWindow(window->sdl_window);
  SDL_Quit();

  bb_assert(window > 0);
  free(window);
}

void catchEvents(Window* window) {
  for (unsigned int i = 0; i < NUM_KEYS; i++) {
    window->input.keys_down[i] = false;
    window->input.keys_up[i] = false;
  }

  for (unsigned int i = 0; i < NUM_MOUSE_BUTTONS; i++) {
    window->input.buttons_down[i] = false;
    window->input.buttons_up[i] = false;
  }

  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    ImGui_ImplSdlGL3_ProcessEvent(&ev);
    if (ev.type == SDL_QUIT) {
      window->is_close_requested = true;
    } else if (ev.type == SDL_MOUSEMOTION) {
      window->input.mouse_x = ev.motion.x;
      window->input.mouse_y = ev.motion.y;
    } else if (ev.type == SDL_KEYDOWN) {
      uint32_t value = ev.key.keysym.scancode;

      window->input.keys[value] = true;
      window->input.keys_down[value] = true;
    } else if (ev.type == SDL_KEYUP) {
      uint32_t value = ev.key.keysym.scancode;

      window->input.keys[value] = false;
      window->input.keys_up[value] = true;
    } else if (ev.type == SDL_MOUSEBUTTONDOWN) {
      uint32_t value = ev.button.button;

      window->input.buttons[value] = true;
      window->input.buttons_down[value] = true;
    } else if (ev.type == SDL_MOUSEBUTTONUP) {
      uint32_t value = ev.button.button;

      window->input.buttons[value] = false;
      window->input.buttons_up[value] = true;
    } else if (ev.type == SDL_WINDOWEVENT) {
      if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
        window->width = ev.window.data1;
        window->height = ev.window.data2;
      }
    }
  }
}

void swapBuffers(Window* window) {
  SDL_GL_SwapWindow(window->sdl_window);
}
