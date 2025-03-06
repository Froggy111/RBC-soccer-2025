#include "camera.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <libcamera/camera_manager.h>
#include <libcamera/camera.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/stream.h>
#include <libcamera/formats.h>

Camera::Camera() : initialized(false), capturing(false) {
    currentConfig = CameraConfig();
}

Camera::~Camera() {
    if (initialized) {
        release();
    }
}

bool Camera::initialize() {
    if (initialized) {
        return true;
    }

    cameraManager = std::make_unique<libcamera::CameraManager>();
    int ret = cameraManager->start();
    if (ret) {
        std::cerr << "Failed to start camera manager: " << ret << std::endl;
        return false;
    }

    if (cameraManager->cameras().empty()) {
        std::cerr << "No cameras available" << std::endl;
        cameraManager->stop();
        return false;
    }

    // Get the first camera
    camera = cameraManager->cameras()[0];
    ret = camera->acquire();
    if (ret) {
        std::cerr << "Failed to acquire camera: " << ret << std::endl;
        cameraManager->stop();
        return false;
    }

    setupCamera();
    initialized = true;
    return true;
}

void Camera::setupCamera() {
    cameraConfig = camera->generateConfiguration({ libcamera::StreamRole::Viewfinder });
    configureCamera();
    
    if (camera->configure(cameraConfig.get()) < 0) {
        std::cerr << "Failed to configure camera" << std::endl;
        return;
    }
    
    mapBuffers();
}

void Camera::configureCamera() {
    // Configure the first stream (we expect to have at least one)
    libcamera::StreamConfiguration& streamConfig = cameraConfig->at(0);
    streamConfig.size.width = currentConfig.width;
    streamConfig.size.height = currentConfig.height;
    streamConfig.pixelFormat = libcamera::formats::RGB888;
    streamConfig.bufferCount = 4;
    
    // Set the frame rate
    auto& controls = cameraConfig->controls();
    controls.set(libcamera::controls::FrameDurationLimits,
                 libcamera::Span<const int64_t, 2>({ 1000000 / currentConfig.fps, 1000000 / currentConfig.fps }));
    
    // Set exposure settings
    if (!currentConfig.autoExposure) {
        controls.set(libcamera::controls::AeEnable, false);
        controls.set(libcamera::controls::ExposureTime, currentConfig.exposureTime);
        controls.set(libcamera::controls::AnalogueGain, currentConfig.analogGain);
    } else {
        controls.set(libcamera::controls::AeEnable, true);
    }
}

void Camera::mapBuffers() {
    // Implementation for mapping buffers would go here
    // This is a simplified version and would need to be expanded
    // based on specific needs
}

void Camera::unmapBuffers() {
    mappedBuffers.clear();
}

void Camera::release() {
    if (!initialized) {
        return;
    }
    
    if (capturing) {
        stopCapture();
    }
    
    unmapBuffers();
    
    if (camera) {
        camera->release();
        camera.reset();
    }
    
    if (cameraManager) {
        cameraManager->stop();
        cameraManager.reset();
    }
    
    initialized = false;
}

bool Camera::isInitialized() const {
    return initialized;
}

bool Camera::startCapture() {
    if (!initialized || capturing) {
        return false;
    }
    
    int ret = camera->start();
    if (ret) {
        std::cerr << "Failed to start capture: " << ret << std::endl;
        return false;
    }
    
    capturing = true;
    return true;
}

bool Camera::stopCapture() {
    if (!initialized || !capturing) {
        return false;
    }
    
    camera->stop();
    capturing = false;
    return true;
}

std::vector<uint8_t> Camera::captureFrame() {
    if (!initialized || !capturing) {
        return {};
    }
    
    // This is a simplified version. In a real implementation,
    // you would use the camera's request mechanism to capture frames
    // and properly handle the request completion events.
    
    // For now, return an empty vector
    return {};
}

void Camera::configure(const CameraConfig& config) {
    bool wasCapturing = capturing;
    
    if (wasCapturing) {
        stopCapture();
    }
    
    currentConfig = config;
    
    if (initialized) {
        configureCamera();
        camera->configure(cameraConfig.get());
    }
    
    if (wasCapturing) {
        startCapture();
    }
}

Camera::CameraConfig Camera::getConfig() const {
    return currentConfig;
}

std::string Camera::getCameraInfo() const {
    if (!initialized || !camera) {
        return "Camera not initialized";
    }
    
    return camera->id() + " - " + camera->properties().get(libcamera::properties::Model);
}