#include <SDL2/SDL.h>
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <stdint.h>
#include <fmt/core.h>
#include "lib/display.hpp"

namespace display {
  using namespace fmt;
  using namespace std;

  Display::Display(int width, int height, std::string window_name, bool use_renderer, int renderer_idx, uint32_t renderer_flags) {
    window = NULL;
    surface = NULL;
    renderer = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      result::panic(fmt::format("SDL could not initialize. SDL_Error: {}", SDL_GetError()), __PRETTY_FUNCTION__);
      return;
    }
    window = SDL_CreateWindow(window_name.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, false);
    if (window == NULL) {
      result::panic(fmt::format("SDL window could not be created. SDL_Error: {}", SDL_GetError()), __PRETTY_FUNCTION__);
      return;
    }
    if (use_renderer) {
      renderer = SDL_CreateRenderer(window, renderer_idx, renderer_flags);
      if (renderer == NULL) {
        result::panic(fmt::format("SDL renderer could not be created. SDL_Error: {}", SDL_GetError()), __PRETTY_FUNCTION__);
        return;
      }
    }
    else {
      surface = SDL_GetWindowSurface(window);
      if (surface == NULL) {
        result::panic(fmt::format("SDL screen surface could not be created. SDL_Error: {}", SDL_GetError()), __PRETTY_FUNCTION__);
        return;
      }
    }
    event = new SDL_Event();
    w = width;
    h = height;
    return;
  }
}