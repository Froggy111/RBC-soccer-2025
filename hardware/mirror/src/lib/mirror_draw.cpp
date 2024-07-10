#include "lib/mirror_stuff.hpp"
#include "lib/display.hpp"
#include "lib/result.hpp"
#include <vector>
#include <array>
#include <stdint.h>
#include <iostream>
#include <fmt/core.h>

using namespace std;

namespace mirror {
  /// @brief draws a 2D mirror slice on an SDL renderer. Mirror points should be sequential
  /// @param plotted_mirror 2D mirror points
  /// @param disp SDL display. Should be initialised with renderer.
  void draw_2D_mirror(
    vector<array<double, 2>> const &plotted_mirror,
    display::Display disp
  ) {
    float mirror_width = plotted_mirror[0][0] * 2;
    float scale = disp.w / mirror_width;
    cout << "scale: " << scale << endl;
    float x_offset = plotted_mirror[0][0] * scale;
    cout << "x_offset: " << scale << endl;
    float y_offset = plotted_mirror[0][1];
    cout << "y_offset: " << scale << endl;
    float x, y;
    float last_x, last_y;

    SDL_FPoint* offset_points;
    offset_points = (SDL_FPoint*) malloc(plotted_mirror.size() * sizeof(SDL_FPoint));

    if (!offset_points) {
      result::panic("Malloc failed in draw 2D mirror.", __PRETTY_FUNCTION__);
      return;
    }

    for (int i = 0; i < plotted_mirror.size(); i++) {
      x = plotted_mirror[i][0] * scale + x_offset;
      y = (y_offset - plotted_mirror[i][1]) * scale;
      offset_points[i] = {x, y};
      // cout << fmt::format("Scaled point {}: x = {}, y = {}", i, offset_points[i].x, offset_points[i].y) << endl;
    }

    int success = SDL_RenderDrawLinesF(disp.renderer, offset_points, plotted_mirror.size());
    cout << success << endl;

    free(offset_points);

    return;
  }
}