#pragma once

#include <string>
#include <memory>

class Camera {
public:
    Camera();
    ~Camera();
    
    // Initialize camera with specified resolution
    bool init(int width = 640, int height = 480, int fps = 30);
    
    // Capture a new frame
    bool update();
    
    // Get camera status (0 = error, 1 = initialized but not capturing, 2 = capturing)
    int getStatus() const;
    
    // Save current frame to file
    bool saveFrame(const std::string& filename) const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;  // PIMPL idiom to hide implementation details
    
    int width_;
    int height_;
    int fps_;
    int status_;
};