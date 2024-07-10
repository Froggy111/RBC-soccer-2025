#pragma once
#include <vector>
#include <array>
#include <functional>
#include "lib/display.hpp"

namespace mirror {
  /// @brief generates 2D mirror slice for a flat surface mapped by some function. Assumes symmetry around y axis.\
  Field is taken to be y=0.
  /// @param camera_y y value of camera focal point from field.
  /// @param fov camera FOV. Do not divide by 2.
  /// @param pixels_per_unit number of points to generate per unit of camera_y
  /// @param steps_per_pixel number of steps to run for each pixel. Increase to improve computational accuracy.
  /// @param theta_to_dist_fn function that takes in a value from 0 to 1 (theta / max theta)\
  and outputs a value from 0 to 1 (dist / max dist).\
  For example, if distance camera sees is wanted to be = k * distance, this function should simply return the input.\
  This is to allow things such as magnifying areas closer to the bot.
  /// @param mirror_y y value of edge of mirror from field.
  /// @param max_x maximum x value of the line wanted to be mirrored.\
  This would be half the field length when bot is placed in the middle.\
  Recommended to put as the maximum this value can possibly be.
  /// @param distribution_grad the magnification bias of the theta_to_dist_fn. 1 = directly proportional
  /// @return array of 2d points on the mirror surface.
  std::vector<std::array<double, 2>> map_points_2D(
    float camera_y, float fov, int pixels_per_unit, int steps_per_pixel,
    std::function<double(double, float)> theta_to_dist_fn, float mirror_y,
    float max_x, float distribution_grad
  );

  std::array<double, 2> get_mirror_grad_2D(
    std::array<double, 2> left_pos, std::array<double, 2> reflect_pos, std::array<double, 2> right_pos
  );

  /// @brief draws a 2D mirror slice on an SDL renderer. Mirror points should be sequential
  /// @param plotted_mirror 2D mirror points
  /// @param disp SDL display. Should be initialised with renderer.
  void draw_2D_mirror(
    std::vector<std::array<double, 2>> const &plotted_mirror,
    display::Display disp
  );
}