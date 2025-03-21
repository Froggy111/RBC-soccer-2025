#pragma once

#include <opencv2/opencv.hpp>
#include <libcamera/libcamera.h>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>
#include "config.hpp"


class Camera {
public:
    using FrameProcessor = std::function<void(const cv::Mat&)>;
    
    Camera();
    ~Camera();
    
    // Initialize camera with 480p resolution by default
    bool initialize(cam_config::Resolutions resolution);
    
    // Start capturing frames with optional processing callback
    bool startCapture(FrameProcessor processor = nullptr);
    
    // Stop capturing frames
    void stopCapture();
    
    // Check if camera is running
    bool isRunning() const;
    
    // Get the latest captured frame
    cv::Mat getLatestFrame() const;
    
private:
    // LibCamera objects
    std::unique_ptr<libcamera::CameraManager> cameraManager_;
    std::shared_ptr<libcamera::Camera> camera_;
    std::unique_ptr<libcamera::CameraConfiguration> config_;
    std::unique_ptr<libcamera::FrameBufferAllocator> allocator_;
    std::vector<std::unique_ptr<libcamera::Request>> requests_;
    
    // Capture state
    std::atomic<bool> running_{false};
    std::thread captureThread_;
    FrameProcessor frameProcessor_;
    
    // Frame management
    mutable std::mutex frameMutex_;
    std::condition_variable frameCondition_;
    std::queue<cv::Mat> frameQueue_;
    cv::Mat latestFrame_;
    
    // Camera configuration
    int width_;
    int height_;
    
    // Private methods
    void captureThreadFunc();
    void requestComplete(libcamera::Request* request);
    cv::Mat convertToMat(libcamera::FrameBuffer* buffer);
};