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

    // Create a camera processor instance
    camera::CamProcessor processor;

    // Process each frame
    cv::Mat frame;
    int frame_count   = 0;
    double total_time = 0.0;

    std::cout << "Processing frames..." << std::endl;

    // Process the video
    while (cap.read(frame)) {
        // Time the get_points function
        auto start_time = std::chrono::high_resolution_clock::now();

        auto points = processor.grid_search(frame);

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                            end_time - start_time)

                            .count();
        total_time += duration;

        // Write data to file in CSV format (including timing)
        output_file << frame_count << "," << points.first.x << ","
                    << points.first.y << "," << points.first.heading << ","
                    << points.second << "," << duration << std::endl;

        // Print timing information for this frame
        std::cout << "Frame " << frame_count << ": get_points() took "
                  << duration
                  << " ms (avg: " << (total_time / (frame_count + 1)) << " ms)"
                  << std::endl;

        frame_count++;
        break;
    }

    // Close the output file
    output_file.close();

    // Print timing summary
    double avg_time = total_time / frame_count;
    std::cout << "\nPerformance summary:" << std::endl;
    std::cout << "  Total frames processed: " << frame_count << std::endl;
    std::cout << "  Total processing time: " << total_time << " ms"
              << std::endl;
    std::cout << "  Average time per frame: " << avg_time << " ms" << std::endl;
    std::cout << "  Theoretical max FPS: " << (1000.0 / avg_time) << std::endl;
    std::cout << "\nProcessing complete. Data saved to '" << output_path << "'"
              << std::endl;

    return 0;
}