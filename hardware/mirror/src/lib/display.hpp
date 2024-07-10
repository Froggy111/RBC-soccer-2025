#pragma once

#include <SDL2/SDL.h>
#include <stdint.h>
#include <vector>
#include "lib/result.hpp"

namespace display {
  class Display {
  public:
    SDL_Window* window = NULL;
    SDL_Surface* surface = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Event* event = NULL;
    int w = 0;
    int h = 0;

    Display(int width, int height, std::string window_name, bool use_renderer, int renderer_idx = -1, uint32_t renderer_flags = 0);
    Display() {
      window = NULL;
      surface = NULL;
      renderer = NULL;
      event = NULL;
      w = 0;
      h = 0;
    }
    inline void close(void) {
      SDL_DestroyWindow(this->window);
      this->window = NULL;
      this->event = NULL;
      this->surface = NULL;
      this->renderer = NULL;
      SDL_Quit();
      return;
    }
    inline void set_pixel(int x, int y, uint32_t pixel_value) {
      *this->get_ptr_to_pixel(x, y) = pixel_value;
      return;
    }
    inline void set_horizontal_line_memset(int x, int y, uint32_t pixel_value, int length) {
      wmemset((wchar_t*) this->get_ptr_to_pixel(x, y), pixel_value, length);
      return;
    }
    inline void set_horizontal_line(int x, int y, uint32_t pixel_value, int length) {
      uint32_t* start_pos = this->get_ptr_to_pixel(x, y);
      for (int i = 0; i < length; i++) {
        *(start_pos + i) = pixel_value;
      }
      return;
    }
    inline void set_surface_memset(uint32_t pixel_value) {
      set_horizontal_line_memset(0, 0, pixel_value, this->h * this->w);
      return;
    }
    inline void set_surface(uint32_t pixel_value) {
      set_horizontal_line(0, 0, pixel_value, this->h * this->w);
      return;
    }
    inline void update_display(void) {
      SDL_UpdateWindowSurface(this->window);
    }
    inline uint32_t rgba_to_pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
      return (r >> this->surface->format->Rloss) << this->surface->format->Rshift
             | (g >> this->surface->format->Gloss) << this->surface->format->Gshift
             | (b >> this->surface->format->Bloss) << this->surface->format->Bshift
             | ((a >> this->surface->format->Aloss) << this->surface->format->Ashift & this->surface->format->Amask);
    }
    inline uint32_t* get_ptr_to_pixel(int x, int y) {
      return (uint32_t*) (
        (uint8_t*) this->surface->pixels
        + y * this->surface->pitch
        + x * this->surface->format->BytesPerPixel);
    }
  };
}