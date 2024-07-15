#include "lib/STL.hpp"
#include "lib/result.hpp"
#include <iostream>
#include <vector>
#include <array>

void make_cube_STL(int argc, char* argv[]);

int main(int argc, char* argv[]) {
  make_cube_STL(argc, argv);
  return 0;
}

void make_cube_STL(int argc, char* argv[]) {
  using namespace std;
  /*
  ARGS:
  1. file name
  2. STL header
  3. cube scale (float)
  */
  if (argc != 4) {
    result::panic("Invalid args.", __PRETTY_FUNCTION__);
    return;
  }

  string file_name = argv[1];
  string STL_header = argv[2];
  float cube_scale = stof(argv[3]); 

  array<array<array<float, 3>, 3>, 12> triangles;
  triangles[0] = {{{-1, -1, -1}, {1, -1, -1}, {1, 1, -1}}};
  triangles[1] = {{{-1, -1, -1}, {-1, 1, -1}, {1, 1, -1}}};
  triangles[2] = {{{-1, -1, 1}, {1, -1, 1}, {1, 1, 1}}};
  triangles[3] = {{{-1, -1, 1}, {-1, 1, 1}, {1, 1, 1}}};
  triangles[4] = {{{-1, -1, -1}, {-1, 1, -1}, {-1, 1, 1}}};
  triangles[5] = {{{-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}}};
  triangles[6] = {{{1, -1, -1}, {1, 1, -1}, {1, 1, 1}}};
  triangles[7] = {{{1, -1, -1}, {1, -1, 1}, {1, 1, 1}}};
  triangles[8] = {{{-1, -1, -1}, {1, -1, -1}, {1, -1, 1}}};
  triangles[9] = {{{-1, -1, -1}, {-1, -1, 1}, {1, -1, 1}}};
  triangles[10] = {{{-1, 1, -1}, {1, 1, -1}, {1, 1, 1}}};
  triangles[11] = {{{-1, 1, -1}, {-1, 1, 1}, {1, 1, 1}}};

  STL::STLFile stl_file(file_name);
  stl_file.write_header(STL_header);
  vector<STL::TriangleBuffer> stl_triangles;
  for (int i = 0; i < triangles.size(); i++) {
    stl_triangles.push_back(STL::get_triangle_buffer(triangles[i]));
  }
  stl_file.write_triangles(stl_triangles);

  return;
}