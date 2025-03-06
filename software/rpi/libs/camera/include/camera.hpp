#include <libcamera/libcamera.h>
#include <memory>
#include <vector>
#include <string>

class Camera {
public:
    Camera();
    ~Camera();

    struct CameraConfig {
        unsigned int width = 640;
        unsigned int height = 480;
        unsigned int fps = 30;
        bool autoExposure = true;
        int exposureTime = 20000; // in microseconds, used if autoExposure is false
        float analogGain = 1.0f;
    };

    bool initialize();
    void release();
    bool isInitialized() const;
    
    bool startCapture();
    bool stopCapture();
    
    // Capture a frame and return a buffer containing the image data
    std::vector<uint8_t> captureFrame();
    
    // Configure camera settings
    void configure(const CameraConfig& config);
    CameraConfig getConfig() const;
    
    // Get camera information
    std::string getCameraInfo() const;

private:
    bool initialized;
    bool capturing;
    CameraConfig currentConfig;

    std::unique_ptr<libcamera::CameraManager> cameraManager;
    std::shared_ptr<libcamera::Camera> camera;
    std::unique_ptr<libcamera::CameraConfiguration> cameraConfig;
    std::map<libcamera::FrameBuffer *, std::vector<uint8_t>> mappedBuffers;
    
    void setupCamera();
    void configureCamera();
    void mapBuffers();
    void unmapBuffers();
};

#endif // CAMERA_HPP