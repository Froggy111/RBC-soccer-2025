#include <iostream>
#include <fstream>
#include "raspicam.h"
#include <unistd.h>

int main() {
    // Create camera object
    raspicam::RaspiCam camera;

    // Set desired image format and resolution
    camera.setFormat(raspicam::RASPICAM_FORMAT_BGR); // Use BGR for OpenCV compatibility
    camera.setWidth(1024);
    camera.setHeight(768);
    camera.setFrameRate(24);

    // Open camera
    if (!camera.open()) {
        std::cerr << "Error: Could not open the camera" << std::endl;
        return -1;
    }

    // Allow the camera to stabilize (adjust exposure, etc.)
    std::cout << "Waiting for camera to stabilize..." << std::endl;
    sleep(2); // Pause for 2 seconds

    // Allocate memory for the image
    unsigned int imageSize = camera.getImageTypeSize(raspicam::RASPICAM_FORMAT_BGR);
    unsigned char* data = new unsigned char[imageSize];

    // Capture an image
    camera.grab();
    camera.retrieve(data, raspicam::RASPICAM_FORMAT_IGNORE);

    // Save the image to a file
    std::ofstream outFile("raspicam_image.jpg", std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Could not open file for writing" << std::endl;
        delete[] data;
        camera.release();
        return -1;
    }
    outFile.write(reinterpret_cast<char*>(data), imageSize);
    outFile.close();

    std::cout << "Image captured and saved as raspicam_image.jpg" << std::endl;

    // Clean up
    delete[] data;
    camera.release();

    return 0;
}
