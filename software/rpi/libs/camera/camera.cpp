#include "camera.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>

// Private implementation structure (PIMPL idiom)
struct Camera::Impl {
    cv::VideoCapture cap;
    bool isInitialized = false;
};

Camera::Camera() : pImpl(new Impl()), width_(0), height_(0), fps_(0), status_(0) {
    std::cout << "Camera instance created" << std::endl;
}

Camera::~Camera() {
    if (pImpl->isInitialized) {
        pImpl->cap.release();
        std::cout << "Camera released" << std::endl;
    }
}

bool Camera::init(int width, int height, int fps) {
    width_ = width;
    height_ = height;
    fps_ = fps;
    
    // Open default camera (usually /dev/video0 on Linux)
    pImpl->cap.open(0);
    
    if (!pImpl->cap.isOpened()) {
        std::cerr << "Error: Could not open camera" << std::endl;
        status_ = 0;
        return false;
    }
    
    // Set camera properties
    pImpl->cap.set(cv::CAP_PROP_FRAME_WIDTH, width_);
    pImpl->cap.set(cv::CAP_PROP_FRAME_HEIGHT, height_);
    pImpl->cap.set(cv::CAP_PROP_FPS, fps_);
    
    pImpl->isInitialized = true;
    status_ = 1; // Initialized
    std::cout << "Camera initialized: " << width_ << "x" << height_ << " @ " << fps_ << "fps" << std::endl;
    
    return true;
}

bool Camera::update() {
    if (!pImpl->isInitialized) {
        std::cerr << "Camera not initialized" << std::endl;
        return false;
    }
    
    if (!pImpl->cap.read(currentFrame_)) {
        std::cerr << "Failed to capture frame" << std::endl;
        status_ = 1; // Problem with capture
        return false;
    }
    
    status_ = 2; // Successfully capturing
    return true;
}

cv::Mat Camera::getFrame() const {
    return currentFrame_;
}

int Camera::getStatus() const {
    return status_;
}

bool Camera::saveFrame(const std::string& filename) const {