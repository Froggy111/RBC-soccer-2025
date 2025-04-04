#include "camera.hpp"
#include "config.hpp"
#include "debug.hpp"
#include <chrono>
#include <iostream>
#include <sys/mman.h>

namespace camera {
Camera::Camera() = default;

Camera::~Camera() {
    stopCapture();

    if (camera_ && isRunning()) {
        camera_->stop();
    }

    allocator_.reset();
    camera_.reset();
    cameraManager_.reset();
}

bool Camera::initialize(camera::Resolutions resolution) {
    switch (resolution) {
        case camera::RES_1232P:
            width_  = 1640;
            height_ = 1232;
            break;
        case camera::RES_1080P:
            width_  = 1920;
            height_ = 1080;
            break;
        case camera::RES_480P:
            width_  = 640;
            height_ = 480;
            break;
        default: std::cerr << "Invalid resolution" << std::endl; return false;
    }

    // Create camera manager
    cameraManager_ = std::make_unique<libcamera::CameraManager>();
    int ret        = cameraManager_->start();
    if (ret) {
        std::cerr << "Failed to start camera manager: " << ret << std::endl;
        return false;
    }

    // Get first available camera
    auto cameras = cameraManager_->cameras();
    if (cameras.empty()) {
        std::cerr << "No cameras available" << std::endl;
        return false;
    }

    camera_ = cameras[0];
    if (!camera_) {
        std::cerr << "Failed to get camera" << std::endl;
        return false;
    }

    // * Acquire camera
    ret = camera_->acquire();
    if (ret) {
        std::cerr << "Failed to acquire camera: " << ret << std::endl;
        return false;
    }

    // * Configure camera
    config_ =
        camera_->generateConfiguration({libcamera::StreamRole::VideoRecording});
    if (!config_) {
        std::cerr << "Failed to generate configuration" << std::endl;
        return false;
    }

    // Set resolution
    config_->at(0).size.width  = width_;
    config_->at(0).size.height = height_;
    config_->at(0).pixelFormat =
        libcamera::formats::RGB888; // Format compatible with OpenCV
    config_->at(0).bufferCount = 4; // Multiple buffers for performance

    // Apply configuration
    ret = camera_->configure(config_.get());
    if (ret) {
        std::cerr << "Failed to configure camera: " << ret << std::endl;
        return false;
    }

    // Set up buffer allocator
    allocator_ = std::make_unique<libcamera::FrameBufferAllocator>(camera_);
    for (libcamera::StreamConfiguration &cfg : *config_) {
        ret = allocator_->allocate(cfg.stream());
        if (ret < 0) {
            std::cerr << "Failed to allocate buffers" << std::endl;
            return false;
        }
    }

    // Create requests
    libcamera::Stream *stream =
        const_cast<libcamera::Stream *>(config_->at(0).stream());
    const std::vector<std::unique_ptr<libcamera::FrameBuffer>> &buffers =
        allocator_->buffers(stream);

    for (unsigned int i = 0; i < buffers.size(); ++i) {
        std::unique_ptr<libcamera::Request> request = camera_->createRequest();
        if (!request) {
            std::cerr << "Failed to create request" << std::endl;
            return false;
        }

        // Use pointer to the FrameBuffer, not moving ownership
        const std::unique_ptr<libcamera::FrameBuffer> &fb = buffers[i];
        int ret = request->addBuffer(stream, fb.get());
        if (ret < 0) {
            std::cerr << "Failed to add buffer to request" << std::endl;
            return false;
        }

        requests_.push_back(std::move(request));
    }

    debug::info("Camera initialized with resolution %dx%d", width_, height_);
    return true;
}

bool Camera::startCapture(FrameProcessor processor) {
    if (running_) {
        std::cerr << "Camera is already running" << std::endl;
        return false;
    }

    if (!camera_) {
        std::cerr << "Camera not initialized" << std::endl;
        return false;
    }

    frameProcessor_ = processor;

    // Clear any existing frames
    {
        std::unique_lock<std::mutex> lock(frameMutex_);
        while (!frameQueue_.empty()) {
            frameQueue_.pop();
        }
        latestFrame_ = cv::Mat();
    }

    // Start camera
    int ret = camera_->start();
    if (ret) {
        std::cerr << "Failed to start camera: " << ret << std::endl;
        return false;
    }

    // Set up completion callback
    camera_->requestCompleted.connect(this, &Camera::requestComplete);

    // Queue initial requests
    for (auto &request : requests_) {
        camera_->queueRequest(request.get());
    }

    running_ = true;

    // Start capture thread
    captureThread_ = std::thread(&Camera::captureThreadFunc, this);

    debug::info("Camera started capturing");
    return true;
}

void Camera::stopCapture() {
    if (!running_) {
        return;
    }

    running_ = false;

    if (captureThread_.joinable()) {
        frameCondition_.notify_all();
        captureThread_.join();
    }

    if (camera_ && isRunning()) {
        camera_->stop();
    }

    debug::info("Camera stopped capturing");
}

bool Camera::isRunning() const { return running_; }

cv::Mat Camera::getLatestFrame() const {
    std::unique_lock<std::mutex> lock(frameMutex_);
    return latestFrame_.clone();
}

void Camera::captureThreadFunc() {
    while (running_) {
        std::unique_lock<std::mutex> lock(frameMutex_);
        if (frameQueue_.empty()) {
            // Wait for a frame or stop signal
            frameCondition_.wait_for(
                lock, std::chrono::milliseconds(100),
                [this] { return !frameQueue_.empty() || !running_; });

            if (!running_) {
                break;
            }

            if (frameQueue_.empty()) {
                continue;
            }
        }

        // Get next frame from queue
        cv::Mat frame = frameQueue_.front();
        frameQueue_.pop();

        // Update latest frame
        latestFrame_ = frame.clone();

        // Release lock before processing
        lock.unlock();

        // Process frame if callback provided
        if (frameProcessor_) {
            frameProcessor_(frame);
        }
    }
}

void Camera::requestComplete(libcamera::Request *request) {
    if (request->status() == libcamera::Request::RequestCancelled) {
        return;
    }

    // Get frame buffer from completed request
    const libcamera::Stream *stream = config_->at(0).stream();
    libcamera::FrameBuffer *buffer  = request->buffers().at(stream);

    // Convert to OpenCV Mat
    cv::Mat frame = convertToMat(buffer);

    // Add to frame queue
    {
        std::unique_lock<std::mutex> lock(frameMutex_);
        // Limit queue size - discard oldest frames if queue gets too large
        if (frameQueue_.size() > 10) {
            frameQueue_.pop();
        }
        frameQueue_.push(frame);
        frameCondition_.notify_one();
    }

    // Re-queue request for continuous capture
    request->reuse();
    request->addBuffer(stream, buffer);
    camera_->queueRequest(request);
}

cv::Mat Camera::convertToMat(libcamera::FrameBuffer *buffer) {
    // Get the first plane (RGB data is in a single plane for RGB888 format)
    const libcamera::FrameBuffer::Plane &plane = buffer->planes()[0];

    // Map the buffer memory
    void *data = mmap(nullptr, plane.length, PROT_READ, MAP_SHARED, plane.fd.get(), 0);

    if (data == MAP_FAILED) {
        std::cerr << "Failed to mmap buffer" << std::endl;
        return cv::Mat();
    }

    cv::Mat image(height_, width_, CV_8UC3, data);
    cv::Mat result = image.clone();
    munmap(data, plane.length);
    return result;
}
} // namespace camera