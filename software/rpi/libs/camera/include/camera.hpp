#include <opencv2/opencv.hpp>
#include <libcamera/libcamera.h>

class Camera {
  public:
    Camera();
    ~Camera();
    cv::Mat getFrame();
};