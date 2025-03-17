#include "lib/math_utils.hpp"
#include "lib/mirror_stuff.hpp"
#include <array>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

namespace mirror {
/// @brief Generates a DXF file from a mirror slice for CAD import
/// @param points The vector of 2D points representing the mirror slice
/// @param filename The name of the output DXF file
void generate_DXF(const std::vector<std::array<double, 2>> &points,
                  const std::string &filename) {
  std::ofstream dxf_file(filename);

  if (!dxf_file.is_open()) {
    std::cerr << "Error: Unable to open file " << filename << " for writing."
              << std::endl;
    return;
  }

  // Write DXF header
  dxf_file << "0\nSECTION\n";
  dxf_file << "2\nHEADER\n";
  dxf_file << "0\nENDSEC\n";

  // Write entities section
  dxf_file << "0\nSECTION\n";
  dxf_file << "2\nENTITIES\n";

  // Write polyline entity
  dxf_file << "0\nPOLYLINE\n";
  dxf_file << "8\n0\n";  // Layer
  dxf_file << "66\n1\n"; // Vertices follow
  dxf_file << "70\n1\n"; // Closed polyline

  // Write vertices
  for (const auto &point : points) {
    dxf_file << "0\nVERTEX\n";
    dxf_file << "8\n0\n"; // Layer
    dxf_file << "10\n"
             << std::fixed << std::setprecision(6) << point[0] << "\n"; // X
    dxf_file << "20\n"
             << std::fixed << std::setprecision(6) << point[1] << "\n"; // Y
    dxf_file << "30\n0.0\n";                                            // Z
  }

  // End polyline
  dxf_file << "0\nSEQEND\n";

  // End entities section
  dxf_file << "0\nENDSEC\n";

  // Write EOF
  dxf_file << "0\nEOF\n";

  dxf_file.close();
  std::cout << "DXF file successfully generated: " << filename << std::endl;
}

/// @brief Generates a DXF file from a mirror slice with additional options
/// @param points The vector of 2D points representing the mirror slice
/// @param filename The name of the output DXF file
/// @param mirror_around_y Whether to mirror the slice around the y-axis
/// @param include_axis Whether to include the axis lines for reference
/// @param scale Scale factor for the exported geometry
void generate_DXF_extended(const std::vector<std::array<double, 2>> &points,
                           const std::string &filename, bool mirror_around_y,
                           bool include_axis, double scale) {
  std::vector<std::array<double, 2>> export_points = points;

  // Scale points if needed
  if (scale != 1.0) {
    for (auto &point : export_points) {
      point[0] *= scale;
      point[1] *= scale;
    }
  }

  std::ofstream dxf_file(filename);

  if (!dxf_file.is_open()) {
    std::cerr << "Error: Unable to open file " << filename << " for writing."
              << std::endl;
    return;
  }

  // Write DXF header
  dxf_file << "0\nSECTION\n";
  dxf_file << "2\nHEADER\n";
  dxf_file << "0\nENDSEC\n";

  // Write entities section
  dxf_file << "0\nSECTION\n";
  dxf_file << "2\nENTITIES\n";

  // Write polyline entity for the mirror slice
  dxf_file << "0\nPOLYLINE\n";
  dxf_file << "8\nMirror\n"; // Layer
  dxf_file << "66\n1\n";     // Vertices follow
  dxf_file << "70\n"
           << (mirror_around_y ? "1" : "0") << "\n"; // Closed if mirroring

  // Write vertices
  for (const auto &point : export_points) {
    dxf_file << "0\nVERTEX\n";
    dxf_file << "8\nMirror\n"; // Layer
    dxf_file << "10\n"
             << std::fixed << std::setprecision(6) << point[0] << "\n"; // X
    dxf_file << "20\n"
             << std::fixed << std::setprecision(6) << point[1] << "\n"; // Y
    dxf_file << "30\n0.0\n";                                            // Z
  }

  // If mirroring, add mirrored points
  if (mirror_around_y) {
    for (int i = export_points.size() - 1; i >= 0; i--) {
      dxf_file << "0\nVERTEX\n";
      dxf_file << "8\nMirror\n"; // Layer
      dxf_file << "10\n"
               << std::fixed << std::setprecision(6) << -export_points[i][0]
               << "\n"; // -X
      dxf_file << "20\n"
               << std::fixed << std::setprecision(6) << export_points[i][1]
               << "\n";        // Y
      dxf_file << "30\n0.0\n"; // Z
    }
  }

  // End polyline
  dxf_file << "0\nSEQEND\n";

  // Add axis lines if requested
  if (include_axis) {
    // X-axis
    dxf_file << "0\nLINE\n";
    dxf_file << "8\nAxis\n";   // Layer
    dxf_file << "10\n-10.0\n"; // Start X
    dxf_file << "20\n0.0\n";   // Start Y
    dxf_file << "30\n0.0\n";   // Start Z
    dxf_file << "11\n10.0\n";  // End X
    dxf_file << "21\n0.0\n";   // End Y
    dxf_file << "31\n0.0\n";   // End Z

    // Y-axis
    dxf_file << "0\nLINE\n";
    dxf_file << "8\nAxis\n";   // Layer
    dxf_file << "10\n0.0\n";   // Start X
    dxf_file << "20\n-10.0\n"; // Start Y
    dxf_file << "30\n0.0\n";   // Start Z
    dxf_file << "11\n0.0\n";   // End X
    dxf_file << "21\n10.0\n";  // End Y
    dxf_file << "31\n0.0\n";   // End Z
  }

  // End entities section
  dxf_file << "0\nENDSEC\n";

  // Write EOF
  dxf_file << "0\nEOF\n";

  dxf_file.close();
  std::cout << "DXF file successfully generated: " << filename << std::endl;
}
} // namespace mirror
