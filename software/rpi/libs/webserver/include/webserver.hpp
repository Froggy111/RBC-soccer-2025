#pragma once

#include <opencv2/opencv.hpp>

extern "C" {
#include <mutex>
#include <libwebsockets.h>
}

#include <atomic>
#include <thread>

/**
 * @class WebServer
 * @brief Simple HTTP server for streaming camera frames as MJPEG.
 */
class WebServer {
public:
    /**
     * @brief Constructor
     * @param port Port number to listen on
     * @param fps Maximum frames per second to stream
     */
    WebServer(int port = 8080, int fps = 30);
    
    /**
     * @brief Destructor - stops server if running
     */
    ~WebServer();
    
    /**
     * @brief Starts the web server in a separate thread
     * @return true if started successfully
     */
    bool start();
    
    /**
     * @brief Stops the web server
     */
    void stop();
    
    /**
     * @brief Updates the frame to be streamed
     * @param frame OpenCV Mat frame to stream
     */
    void updateFrame(const cv::Mat& frame);
    
    /**
     * @brief Checks if the server is running
     * @return true if the server is running
     */
    bool isRunning() const;

    /**
     * @brief Get the current viewer count
     * @return Number of connected viewers
     */
    int getViewerCount() const;

private:
    // Server configuration
    int m_port;
    int m_fps;
    std::atomic<bool> m_running;
    std::atomic<int> m_viewerCount;
    
    // Frame data
    cv::Mat m_currentFrame;
    std::vector<uchar> m_jpegBuffer;
    std::mutex m_frameMutex;
    
    // Server thread
    std::unique_ptr<std::thread> m_serverThread;
    
    // Implementation-specific private methods
    struct lws_context *m_context;
    void serverLoop();
    static int handleLibwebsocketEvent(struct lws *wsi, enum lws_callback_reasons reason, 
                                      void *user, void *in, size_t len);
};