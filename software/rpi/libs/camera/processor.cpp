#include "processor.hpp"
#include "field.hpp"
#include <cstdint>

namespace camera {
void CamProcessor::process_frame(const cv::Mat &frame) {}

void CamProcessor::calculate_loss(const cv::Mat &frame) {
    int16_t white_lines_coords[WHITE_LINES_LENGTH];
    memcpy(white_lines_coords, WHITE_LINES, sizeof(white_lines_coords));

	// * transform & rotate white line coords
    for (int i = 0; i < WHITE_LINES_LENGTH; i++) {
        int16_t x = std::get<0>(white_lines_coords[i]);
        int16_t y = std::get<1>(white_lines_coords[i]);

        // transform
	}
}

}