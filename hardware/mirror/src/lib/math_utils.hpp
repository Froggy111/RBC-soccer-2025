#pragma once
#include <vector>
#include <array>
#include <cmath>

namespace math_utils {
  template <int size>
  inline double get_length(std::array<double, size> vec);

  template <int size>
  inline std::array<double, size> normalise(std::array<double, size> vec) {
    double len = get_length<size>(vec);
    std::array<double, size> norm_vec;
    for (int i = 0; i < size; i++) {
      norm_vec[i] = vec[i] / len;
    }
    return norm_vec;
  }

  template <int size>
  inline double get_length(std::array<double, size> vec) {
    double sum = 0;
    for (double val : vec) {
      sum += val * val;
    }
    return std::sqrt(sum);
  }

  template <int size>
  inline std::array<double, size> subtract(std::array<double, size> a, std::array<double, size> b) {
    std::array<double, size> subtracted;
    for (int i = 0; i < size; i++) {
      subtracted[i] = a[i] - b[i];
    }
    return subtracted;
  }

  template <int size>
  inline std::array<double, size> add(std::array<double, size> a, std::array<double, size> b) {
    std::array<double, size> added;
    for (int i = 0; i < size; i++) {
      added[i] = a[i] + b[i];
    }
    return added;
  }

  template <int size>
  inline std::array<double, size> multiply(std::array<double, size> a, double b) {
    std::array<double, size> multiplied;
    for (int i = 0; i < size; i++) {
      multiplied[i] = a[i] * b;
    }
    return multiplied;
  }

  inline std::array<double, 3> rotate_point_around_y(std::array<double, 3> point, double sintheta, double costheta) {
    double x = point[0] * costheta - point[2] * sintheta;
    double z = point[0] * sintheta + point[2] * costheta;
    return {x, point[1], z};
  }

  inline void rotate_points_around_y(std::vector<std::array<double, 3>> &points, double sintheta, double costheta) {
    #pragma omp parallel for
    for (int i = 0; i < points.size(); i++) {
      points[i] = rotate_point_around_y(points[i], sintheta, costheta);
    }
    return;
  }
}