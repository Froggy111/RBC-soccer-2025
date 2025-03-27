#include "field.hpp"
#include "position.hpp"
#include "processor.hpp"
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <string>

// Function to read the first frame from a video file
cv::Mat readFirstFrame(const std::string &video_path) {
    cv::VideoCapture cap(video_path);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file " << video_path
                  << std::endl;
        return cv::Mat();
    }

    cv::Mat frame;
    cap >> frame;
    return frame;
}

// Function to clamp a value between min and max
float clamp(float value, float min_val, float max_val) {
    return std::max(min_val, std::min(value, max_val));
}

int main(int argc, char **argv) {
    const std::string video_path = "480p.mp4";

    // Initialize processor
    camera::CamProcessor processor;

    // Read the first frame
    std::cout << "Reading first frame from " << video_path << "..."
              << std::endl;
    cv::Mat test_frame = readFirstFrame(video_path);
    if (test_frame.empty()) {
        std::cerr << "Failed to read the first frame. Exiting." << std::endl;
        return 1;
    }

    std::cout << "Frame dimensions: " << test_frame.cols << "x"
              << test_frame.rows << std::endl;

    // Timer for overall performance measurement
    auto start_time = std::chrono::high_resolution_clock::now();

    // Create output directory for frames
    system("mkdir -p heatmap_frames");

    // Process each heading angle (0 to 359 degrees)
    for (int angle = 0; angle < 360; angle++) {
        std::cout << "Generating heatmap for heading " << angle << " degrees..."
                  << std::endl;

        // Convert angle to radians
        float heading_rad = angle * M_PI / 180.0f;

        // Create empty image for the heatmap
        cv::Mat heatmap(camera::FIELD_Y_SIZE, camera::FIELD_X_SIZE, CV_8UC3,
                        cv::Scalar(0, 0, 0));

        // Process each pixel/position in the field
        for (int y = 0; y < camera::FIELD_Y_SIZE; y++) {
            for (int x = 0; x < camera::FIELD_X_SIZE; x++) {
                Pos position(x - camera::FIELD_X_SIZE / 2,
                             y - camera::FIELD_Y_SIZE / 2, heading_rad);

                float loss = processor.calculate_loss(test_frame, position);

                // Ensure loss is between 0 and 1
                loss = clamp(loss, 0.0f, 1.0f);

                // Convert loss to a color (blue to red spectrum)
                // Low loss (good match) = blue, high loss (bad match) = red
                int intensity = static_cast<int>((1.0f - loss) * 255);

                heatmap.at<cv::Vec3b>(y, camera::FIELD_X_SIZE - x) =
                    cv::Vec3b(intensity, intensity, intensity);
            }
        }

        // Apply color map for better visualization
        cv::Mat colored_heatmap;
        cv::applyColorMap(heatmap, colored_heatmap, cv::COLORMAP_JET);

        // Add heading text to the image
        std::stringstream ss;
        ss << "Heading: " << std::setw(3) << angle;
        cv::putText(colored_heatmap, ss.str(), cv::Point(10, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255),
                    2);

        // Save the frame
        std::stringstream filename;
        filename << "heatmap_frames/frame_" << std::setw(3) << std::setfill('0')
                 << angle << ".png";
        cv::imwrite(filename.str(), colored_heatmap);

        // Progress indicator for overall completion
        if (angle % 36 == 0) { // Every 10% of frames
            auto current_time = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                               current_time - start_time)
                               .count();
            std::cout << "Overall progress: " << (angle * 100 / 360)
                      << "% ";
            std::cout << "(Elapsed time: " << elapsed << " seconds)"
                      << std::endl;
        }
    }

    // Calculate total elapsed time
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration =
        std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time)
            .count();
    std::cout << "All heatmaps generated in " << total_duration << " seconds."
              << std::endl;

    // Create a video from the frames
    std::cout << "Creating video from frames..." << std::endl;
    std::string ffmpeg_cmd =
        "ffmpeg -y -framerate 30 -i heatmap_frames/frame_%03d.png -c:v libx264 "
        "-pix_fmt yuv420p heatmap_video.mp4";
    int result = system(ffmpeg_cmd.c_str());

    if (result == 0) {
        std::cout << "Video created successfully: heatmap_video.mp4"
                  << std::endl;
    } else {
        std::cerr << "Failed to create video. Make sure ffmpeg is installed."
                  << std::endl;
    }

    std::cout << "All operations completed successfully." << std::endl;

    return 0;
}