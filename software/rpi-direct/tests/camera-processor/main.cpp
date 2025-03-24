#include "field.hpp"
#include "position.hpp"
#include "processor.hpp"
#include <chrono> // For timing measurements
#include <fstream>
#include <iomanip>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

int main() {
    srand(4);

    const std::string input_path  = "./480p.mp4";
    const std::string output_path = "./points_data.csv";
    cv::VideoCapture cap(input_path);

    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open input video file: " << input_path
                  << std::endl;
        return -1;
    }

    // Get video properties for logging
    int frame_width  = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps       = cap.get(cv::CAP_PROP_FPS);

    std::cout << "Video properties:" << std::endl;
    std::cout << "Width: " << frame_width << std::endl;
    std::cout << "Height: " << frame_height << std::endl;
    std::cout << "FPS: " << fps << std::endl;

    // Open output file
    std::ofstream output_file(output_path);
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not create output file: " << output_path
                  << std::endl;
        return -1;
    }

    output_file << "Frame, X, Y, Heading, Loss, Time (ms)" << std::endl;

    // Set precision for floating point values
    output_file << std::fixed << std::setprecision(4);

    // Prepare for processing
    camera::CamProcessor processor;
    cv::Mat frame;
    int frame_count   = 1;
    double total_time = 0.0;

    // Process the first frame
    Pos center(5, camera::FIELD_HEIGHT - 5, 55);
    cap.read(frame);
    Pos current_pos = processor.find_minima_smart_search(frame, center).first;
    output_file << frame_count << "," << current_pos.x << ","
                << current_pos.y << "," << current_pos.heading << ","
                << 0.0f << "," << 0.0f << std::endl;

    // Process the video
    while (cap.read(frame)) {
        // Time the get_points function
        auto start_time = std::chrono::high_resolution_clock::now();

        // use regression
        auto points = processor.find_minima_regress(frame, current_pos);

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                            end_time - start_time).count();

        total_time += duration;

        // Write data to file in CSV format (including timing)
        output_file << frame_count << "," << points.first.x << ","
                    << points.first.y << "," << points.first.heading << ","
                    << points.second << "," << duration << std::endl;

        // Print timing information for this frame
        std::cout << "Frame " << frame_count << " took "
                  << duration
                  << " ms (avg: " << (total_time / (frame_count + 1)) << " ms)"
                  << std::endl;

        frame_count++;
    }

    // Close the output file
    output_file.close();
    std::cout << "\nProcessing complete. Data saved to '" << output_path << "'"
              << std::endl;

    return 0;
}