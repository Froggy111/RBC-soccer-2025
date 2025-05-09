#include "lib/display.hpp"
#include "lib/math_utils.hpp"
#include "lib/mirror_stuff.hpp"
#include "lib/result.hpp"
#include <cmath>
#include <iostream>
#include <string>

void make_mirror_slice(int argc, char *argv[]);
void generate_mirror_STL(int argc, char *argv[]);
void generate_mirror_DXF(int argc, char *argv[]);

double distribution_fn(double x, float one_over_grad);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " [stl|dxf|display] [args...]"
              << std::endl;
    return 1;
  }

  std::string mode = argv[1];

  // Remove the first argument (mode) and shift the rest
  argc--;
  for (int i = 1; i < argc; i++) {
    argv[i] = argv[i + 1];
  }

  if (mode == "stl") {
    generate_mirror_STL(argc, argv);
  } else if (mode == "dxf") {
    generate_mirror_DXF(argc, argv);
  } else if (mode == "display") {
    make_mirror_slice(argc, argv);
  } else {
    std::cout << "Unknown mode: " << mode << std::endl;
    std::cout << "Available modes: stl, dxf, display" << std::endl;
    return 1;
  }

  return 0;
}

void make_mirror_slice(int argc, char *argv[]) {
  /*
  ARGS:
  1. camera focus point height (float)
  2. mirror highest point height (float)
  3. x distance from camera focus point to edge of field (float)
  4. camera FOV, in degrees (float)
  5. pixels per unit (int)
  6. steps per pixel (int)
  7. distribution grad (float)
  8. display width (int)
  9. display height (int)
  */
  using namespace std;

  if (argc != 10) {
    result::panic("Invalid args.", __PRETTY_FUNCTION__);
  }

  float cam_y = stof(argv[1]);
  float mirror_y = stof(argv[2]);
  float max_x = stof(argv[3]);
  float fov = stof(argv[4]);
  int pixels_per_unit = stoi(argv[5]);
  int steps_per_pixel = stoi(argv[6]);
  float distribution_grad = stof(argv[7]);
  int disp_w = stoi(argv[8]);
  int disp_h = stoi(argv[9]);

  fov = (fov / 180.0f) * M_PI; // convert to radians

  vector<array<double, 2>> mirror_plotted_points = mirror::map_points_2D(
      cam_y, fov, pixels_per_unit, steps_per_pixel, distribution_fn, mirror_y,
      max_x, distribution_grad);

  // for (int i = 0; i < mirror_plotted_points.size(); i++) {
  //   cout << std::format("Mirror point {}: x = {}, y = {}", i,
  //   mirror_plotted_points[i][0], mirror_plotted_points[i][1]) << endl;
  // }

  display::Display disp(disp_w, disp_h, "mirror 2D", true);

  // set bg to white
  SDL_SetRenderDrawColor(disp.renderer, 255, 255, 255, 255);
  SDL_RenderClear(disp.renderer);

  // draw mirror
  SDL_SetRenderDrawColor(disp.renderer, 0, 0, 0, 255);
  mirror::draw_2D_mirror(mirror_plotted_points, disp);

  SDL_RenderPresent(disp.renderer);

  while (true) {
    if (SDL_PollEvent(disp.event)) {
      if (disp.event->type == SDL_QUIT) {
        disp.close();
        result::panic("User closed window", __PRETTY_FUNCTION__);
        return;
      }
    }
  }
}

void generate_mirror_STL(int argc, char *argv[]) {
  /*
  ARGS:
  1. camera focus point height (float)
  2. mirror highest point height (float)
  3. x distance from camera focus point to edge of field (float)
  4. camera FOV, in degrees (float)
  5. pixels per unit (int)
  6. steps per pixel (int)
  7. distribution grad (float)
  8. number of slices (int)
  9. output file name (char[])
  10. file header (char[])
  */
  using namespace std;

  if (argc != 11) {
    result::panic("Invalid args.", __PRETTY_FUNCTION__);
  }

  float cam_y = stof(argv[1]);
  float mirror_y = stof(argv[2]);
  float max_x = stof(argv[3]);
  float fov = stof(argv[4]);
  int pixels_per_unit = stoi(argv[5]);
  int steps_per_pixel = stoi(argv[6]);
  float distribution_grad = stof(argv[7]);
  int n_slices = stoi(argv[8]);
  string file_name = argv[9];
  string file_header = argv[10];

  fov = (fov / 180.0f) * M_PI; // convert to radians

  vector<array<double, 2>> mirror_plotted_points = mirror::map_points_2D(
      cam_y, fov, pixels_per_unit, steps_per_pixel, distribution_fn, mirror_y,
      max_x, distribution_grad);

  STL::STLFile STL_file(file_name);
  STL_file.write_header(file_header);
  mirror::generate_STL(mirror_plotted_points, STL_file, n_slices);

  return;
}

void generate_mirror_DXF(int argc, char *argv[]) {
  /*
  ARGS:
  1. camera focus point height (float)
  2. mirror highest point height (float)
  3. x distance from camera focus point to edge of field (float)
  4. camera FOV, in degrees (float)
  5. pixels per unit (int)
  6. steps per pixel (int)
  7. distribution grad (float)
  8. output file name (char[])
  9. mirror around y-axis (bool: 0/1)
  10. include axis (bool: 0/1)
  11. scale factor (float)
  */
  using namespace std;
  if (argc != 12) {
    result::panic("Invalid args.", __PRETTY_FUNCTION__);
  }

  float cam_y = stof(argv[1]);
  float mirror_y = stof(argv[2]);
  float max_x = stof(argv[3]);
  float fov = stof(argv[4]);
  int pixels_per_unit = stoi(argv[5]);
  int steps_per_pixel = stoi(argv[6]);
  float distribution_grad = stof(argv[7]);
  string file_name = argv[8];
  bool mirror_around_y = (stoi(argv[9]) != 0);
  bool include_axis = (stoi(argv[10]) != 0);
  float scale = stof(argv[11]);

  fov = (fov / 180.0f) * M_PI; // convert to radians

  vector<array<double, 2>> mirror_plotted_points = mirror::map_points_2D(
      cam_y, fov, pixels_per_unit, steps_per_pixel, distribution_fn, mirror_y,
      max_x, distribution_grad);

  mirror::generate_DXF_extended(mirror_plotted_points, file_name,
                                mirror_around_y, include_axis, scale);

  cout << "DXF file successfully generated: " << file_name << endl;
  return;
}

double distribution_fn(double x, float one_over_grad) {
  return pow(x, one_over_grad);
}
