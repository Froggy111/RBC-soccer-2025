#pragma once

#include <fstream>
#include <vector>
#include <array>
#include <string>

#define HEADER_SIZE 80
#define TRIANGLE_SIZE 50
#define ATTRIBUTE_SIZE 2
#define NORMAL_SIZE 12

namespace STL {
  typedef std::array<uint8_t, TRIANGLE_SIZE> TriangleBuffer;
  TriangleBuffer get_triangle_buffer(std::array<std::array<float, 3>, 3> const &points);
  TriangleBuffer get_triangle_buffer(std::array<std::array<double, 3>, 3> const &points);
  TriangleBuffer get_triangle_buffer(std::array<std::array<float, 3>, 3> const &points, std::array<float, 3> const &normal);
  TriangleBuffer get_triangle_buffer(std::array<std::array<double, 3>, 3> const &points, std::array<double, 3> const &normal);

  class STLFile {
  public:
    STLFile(std::string const &filename);
    ~STLFile();
    void write_header(std::string header);
    void write_triangles(std::vector<TriangleBuffer> const &triangles);
    void modify_triangle(TriangleBuffer const &triangle, int idx);
    TriangleBuffer get_triangle(int idx);
  private:
    std::fstream file;
    uint32_t n_triangles = 0;
    void write_n_triangles(uint32_t n_triangles);
  };
}