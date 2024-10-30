#include "lib/STL.hpp"
#include "lib/result.hpp"
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <fmt/core.h>
#include <stdint.h>

using namespace std;

namespace STL {
  TriangleBuffer get_triangle_buffer(array<array<float, 3>, 3> const &points) {
    TriangleBuffer triangle_buf = {0};
    memcpy((void*) (triangle_buf.data() + NORMAL_SIZE), (void*) points.data(), sizeof(points));
    return triangle_buf;
  }
  TriangleBuffer get_triangle_buffer(array<array<double, 3>, 3> const &points) {
    TriangleBuffer triangle_buf = {0};
    array<array<float, 3>, 3> pointsf;
    for (int i = 0; i < points.size(); i++) {
      for (int j = 0; j < points[i].size(); j++) {
        pointsf[i][j] = (float) points[i][j];
      }
    }
    memcpy((void*) (triangle_buf.data() + NORMAL_SIZE), (void*) pointsf.data(), sizeof(pointsf));
    return triangle_buf;
  }
  TriangleBuffer get_triangle_buffer(array<array<float, 3>, 3> const &points, array<float, 3> const &normal) {
    TriangleBuffer triangle_buf = {0};
    memcpy((void*) (triangle_buf.data() + NORMAL_SIZE), (void*) points.data(), sizeof(points));
    memcpy((void*) triangle_buf.data(), (void*) normal.data(), sizeof(normal));
    return triangle_buf;
  }
  TriangleBuffer get_triangle_buffer(array<array<double, 3>, 3> const &points, array<double, 3> const &normal) {
    TriangleBuffer triangle_buf = {0};
    array<array<float, 3>, 3> pointsf;
    for (int i = 0; i < points.size(); i++) {
      for (int j = 0; j < points[i].size(); j++) {
        pointsf[i][j] = (float) points[i][j];
      }
    }
    memcpy((void*) (triangle_buf.data() + NORMAL_SIZE), (void*) pointsf.data(), sizeof(pointsf));

    array<float, 3> normalf;
    for (int i = 0; i < normal.size(); i++) {
      normalf[i] = (float) normal[i];
    }
    memcpy((void*) triangle_buf.data(), (void*) normalf.data(), sizeof(normalf));
    return triangle_buf;
  }
  STLFile::STLFile(string const &filename) {
    file.open(filename, fstream::in | fstream::out | fstream::binary | fstream::trunc);
    n_triangles = 0;
    return;
  }

  STLFile::~STLFile() {
    file.close();
    return;
  }

  void STLFile::write_header(string header) {
    file.seekp(fstream::beg);
    file.write(header.c_str(), HEADER_SIZE);
    return;
  }

  void STLFile::write_triangles(vector<TriangleBuffer> const &triangles) {
    file.seekp(fstream::beg + HEADER_SIZE + sizeof(uint32_t) + TRIANGLE_SIZE * n_triangles);
    file.write((char*) triangles.data(), triangles.size() * sizeof(TriangleBuffer));
    n_triangles += triangles.size();
    this->write_n_triangles(n_triangles);
    return;
  }

  void STLFile::modify_triangle(TriangleBuffer const &triangle, int idx) {
    if (idx > n_triangles - 1) {
      result::panic(std::format("Tried modifying triangle {} when only {} triangles exist", idx, n_triangles), __PRETTY_FUNCTION__);
    }
    file.seekp(fstream::beg + HEADER_SIZE + sizeof(uint32_t) + TRIANGLE_SIZE * idx);
    file.write((char*) triangle.data(), sizeof(TriangleBuffer));
    return;
  }

  TriangleBuffer STLFile::get_triangle(int idx) {
    if (idx > n_triangles - 1) {
      result::panic(std::format("Tried reading triangle {} when only {} triangles exist", idx, n_triangles), __PRETTY_FUNCTION__);
    }
    TriangleBuffer triangle;
    file.seekg(fstream::beg + HEADER_SIZE + sizeof(uint32_t) + TRIANGLE_SIZE * idx);
    file.read((char*) triangle.data(), sizeof(TriangleBuffer));
    return triangle;
  }

  void STLFile::write_n_triangles(uint32_t n_triangles) {
    file.seekp(fstream::beg + HEADER_SIZE);
    file.write((char*) &n_triangles, sizeof(uint32_t));
    return;
  }
}