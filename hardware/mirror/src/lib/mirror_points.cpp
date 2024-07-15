#include "lib/mirror_stuff.hpp"
#include "lib/math_utils.hpp"
#include "lib/STL.hpp"
#include <vector>
#include <array>
#include <functional>
#include <cmath>
#include <iostream>

using namespace std;
using namespace math_utils;

namespace mirror {
  /// @brief generates 2D mirror slice for a flat surface mapped by some function. Assumes symmetry around y axis. Field is taken to be y=0.
  /// @param camera_y y value of camera focal point from field.
  /// @param fov camera FOV. Do not divide by 2.
  /// @param pixels_per_unit number of pixels per input unit. Increase for higher resolution.
  /// @param steps_per_pixel number of steps to run for each pixel. Increase to improve computational accuracy.
  /// @param theta_to_dist_fn function that takes in a value from 0 to 1 (theta / max theta) and outputs a value from 0 to 1 (dist / max dist). For example, if distance camera sees is wanted to be = k * distance, this function should simply return the input. This is to allow things such as magnifying areas closer to the bot.
  /// @param mirror_y y value of edge of mirror from field.
  /// @param max_x maximum x value of the line wanted to be mirrored. This would be half the field length when bot is placed in the middle. Recommended to put as the maximum this value can possibly be.
  /// @param distribution_grad the magnification bias of the theta_to_dist_fn. 1 = directly proportional
  /// @return array of 2d points on the mirror surface.
  vector<array<double, 2>> map_points_2D(
    float camera_y, float fov, int pixels_per_unit, int steps_per_pixel,
    function<double(double, float)> theta_to_dist_fn, float mirror_y,
    float max_x, float distribution_grad
  ) {
    // preprocessing
    const float one_over_grad = 1 / distribution_grad;
    const double max_theta = fov / 2;
    // start at known point y = 0, at the edge.
    const double start_x = (mirror_y - camera_y) * tan(max_theta);
    array<double, 2> mirror_pos;
    mirror_pos[0] = start_x;
    mirror_pos[1] = mirror_y;

    array<double, 2> field_pos;
    field_pos[0] = max_x;
    field_pos[1] = 0;

    array<double, 2> camera_pos;
    camera_pos[0] = 0;
    camera_pos[1] = camera_y;

    const double step_multi = (1.0f / (double) pixels_per_unit) / (double) steps_per_pixel;
    array<double, 2> grad;

    vector<array<double, 2>> mirror_poses;
    mirror_poses.clear();
    mirror_poses.push_back(mirror_pos);

    // newtons method
    int count = 0;
    double theta = max_theta;
    array<double, 2> cam_to_mirror;
    double field_proportion;
    double cam_proportion;
    while (mirror_pos[0] >= 0.0f) {
      count += 1;

      // update mirror pos
      grad = get_mirror_grad_2D(camera_pos, mirror_pos, field_pos);
      
      array<double, 2> increment = multiply<2>(grad, step_multi);
      mirror_pos = add<2>(mirror_pos, increment);
      if (count >= steps_per_pixel) {
        mirror_poses.push_back(mirror_pos);
        count = 0;
      }

      // update field pos
      cam_to_mirror = subtract<2>(mirror_pos, camera_pos);
      theta = atan(cam_to_mirror[0] / cam_to_mirror[1]);
      cam_proportion = tan(theta) / tan(max_theta);
      field_proportion = theta_to_dist_fn(cam_proportion, one_over_grad);
      field_pos[0] = field_proportion * max_x;
    }

    // correct the last point
    mirror_poses.pop_back();
    double correction_multi = (mirror_pos[0] - 0.0f) / grad[0];
    mirror_pos[1] += correction_multi * grad[1];
    mirror_pos[0] = 0.0f;
    mirror_poses.push_back(mirror_pos);

    // // mirror the other side
    // for (int i = mirror_poses.size() - 1; i >= 0; i--) {
    //   mirror_poses.push_back(
    //     {- mirror_poses[i][0], mirror_poses[i][1]}
    //   );
    // }

    return mirror_poses;
  }

  void generate_STL(vector<array<double, 2>> const &points, STL::STLFile &stl_file, int num_slices) {
    /*
    NORMALS DIE
    TRY ADDING A MIDDLE POINT AND NOT MIRRORING OTHER SIDE IN MAP POINTS 2D
    THEN SPIN FOR 2PI AND DRAW BOTTOM TRIANGLES TO MIDDLE
    */
    const double theta = 2 * M_PI / (double) num_slices;
    vector<array<double, 3>> current_points;
    current_points.resize(points.size());
    #pragma omp parallel for
    for (int i = 0; i < points.size(); i++) {
      current_points[i] = {
        points[i][0],
        points[i][1],
        0
      };
    }
    vector<array<double, 3>> previous_points = current_points;
    vector<array<double, 3>> const starting_points = current_points;
    double sintheta = sin(theta);
    double costheta = cos(theta);
    for (int i = 0; i < num_slices - 1; i++) {
      previous_points = current_points;
      rotate_points_around_y(current_points, sintheta, costheta);
      array<array<double, 3>, 3> triangle;
      vector<STL::TriangleBuffer> stl_triangles;
      stl_triangles.resize(0);
      for (int j = 0; j < current_points.size() - 1; j++) {
        triangle[0] = previous_points[j];
        triangle[1] = previous_points[j + 1];
        triangle[2] = current_points[j + 1];
        stl_triangles.push_back(STL::get_triangle_buffer(triangle));
        triangle[1] = current_points[j + 1];
        triangle[2] = current_points[j];
        stl_triangles.push_back(STL::get_triangle_buffer(triangle));
      }
      triangle[0] = previous_points[0];
      triangle[1] = current_points[0];
      triangle[2] = {0, current_points[0][1], 0};
      stl_triangles.push_back(STL::get_triangle_buffer(triangle));
      stl_file.write_triangles(stl_triangles);
    }

    previous_points = current_points;
    array<array<double, 3>, 3> triangle;
    vector<STL::TriangleBuffer> stl_triangles;
    stl_triangles.resize(0);
    for (int j = 0; j < starting_points.size() - 1; j++) {
      triangle[0] = previous_points[j];
      triangle[1] = previous_points[j + 1];
      triangle[2] = starting_points[j + 1];
      stl_triangles.push_back(STL::get_triangle_buffer(triangle));
      triangle[1] = starting_points[j + 1];
      triangle[2] = starting_points[j];
      stl_triangles.push_back(STL::get_triangle_buffer(triangle));
    }
    triangle[0] = previous_points[0];
    triangle[1] = starting_points[0];
    triangle[2] = {0, starting_points[0][1], 0};
    stl_triangles.push_back(STL::get_triangle_buffer(triangle));
    stl_file.write_triangles(stl_triangles);
    return;
  }

  array<double, 2> get_mirror_grad_2D(
    array<double, 2> left_pos, array<double, 2> reflect_pos, array<double, 2> right_pos
  ) {
    array<double, 2> left_vec = subtract<2>(left_pos, reflect_pos);
    array<double, 2> right_vec = subtract<2>(right_pos, reflect_pos);
    left_vec = normalise<2>(left_vec);
    right_vec = normalise<2>(right_vec);
    array<double, 2> dir_vec = subtract<2>(left_vec, right_vec);
    return normalise<2>(dir_vec);
  }
}