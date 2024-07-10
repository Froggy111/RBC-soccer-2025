#pragma once
#include <vector>
#include <array>
#include <cmath>

namespace math_utils {
  template <int size>
  double get_length(std::array<double, size> vec);

  template <int size>
  std::array<double, size> normalise(std::array<double, size> vec) {
    double len = get_length<size>(vec);
    std::array<double, size> norm_vec;
    for (int i = 0; i < size; i++) {
      norm_vec[i] = vec[i] / len;
    }
    return norm_vec;
  }

  template <int size>
  double get_length(std::array<double, size> vec) {
    double sum = 0;
    for (double val : vec) {
      sum += val * val;
    }
    return std::sqrt(sum);
  }

  template <int size>
  std::array<double, size> subtract(std::array<double, size> a, std::array<double, size> b) {
    std::array<double, size> subtracted;
    for (int i = 0; i < size; i++) {
      subtracted[i] = a[i] - b[i];
    }
    return subtracted;
  }

  template <int size>
  std::array<double, size> add(std::array<double, size> a, std::array<double, size> b) {
    std::array<double, size> added;
    for (int i = 0; i < size; i++) {
      added[i] = a[i] + b[i];
    }
    return added;
  }

  template <int size>
  std::array<double, size> multiply(std::array<double, size> a, double b) {
    std::array<double, size> multiplied;
    for (int i = 0; i < size; i++) {
      multiplied[i] = a[i] * b;
    }
    return multiplied;
  }
}