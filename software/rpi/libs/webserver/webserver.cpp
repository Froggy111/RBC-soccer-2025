#include "webserver.hpp"
#include <cstdio>
#include <libwebsockets.h>
#include <sstream>

// Static HTML content to serve
static const char *html_content =
    "<!DOCTYPE html>"
    "<html>"
    "<head><title>RPI Camera Stream</title></head>"
    "<body>"
    "<h1>RPI Camera Stream</h1>"
    "<img src=\"/stream\" width=\"640\" height=\"480\"/>"
    "</body>"
    "</html>";

// Static protocol definition
struct lws_protocols WebServer::protocols[] = {
    {
        "http",                             // Protocol name
        WebServer::handleLibwebsocketEvent, // Callback function
        0,                                  // User data
        0,                                  // Max frame size / rx buffer
    },
    {
        NULL, NULL, 0, 0 // End of list
    }};

WebServer::WebServer(int port, int fps)
    : m_port(port), m_fps(fps), m_running(false), m_viewerCount(0),
      m_context(nullptr) {
  std::printf("Initializing web server with libwebsockets on port %d\n",
              m_port);
}

WebServer::~WebServer() { stop(); }

bool WebServer::start() {
  if (m_running)
    return false;

  m_running = true;
  m_serverThread = std::make_unique<std::thread>(&WebServer::runServer, this);

  return true;
}

void WebServer::stop() {
  if (!m_running)
    return;

  m_running = false;

  if (m_serverThread && m_serverThread->joinable()) {
    m_serverThread->join();
  }

  m_serverThread.reset();
}

void WebServer::updateFrame(const cv::Mat &frame) {
  if (!m_running || frame.empty())
    return;

  std::lock_guard<std::mutex> lock(m_frameMutex);
  frame.copyTo(m_currentFrame);
}

bool WebServer::isRunning() const { return m_running; }

int WebServer::getViewerCount() const { return m_viewerCount; }

void WebServer::runServer() {
  // info for libwebsocket creation
  struct lws_context_creation_info info;
  memset(&info, 0, sizeof(info));
  info.port = m_port;
  info.protocols = WebServer::protocols; // Assign the protocols
  info.options = 0;

  // create libwebsocket context with those options
  m_context = lws_create_context(&info);

  if (!m_context) {
    std::printf("Failed to create libwebsockets context\n");
    m_running = false;
    return;
  }

  while (m_running) {
    lws_service(m_context, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  lws_context_destroy(m_context);
  m_context = nullptr;
}

int WebServer::handleLibwebsocketEvent(struct lws *wsi,
                                       enum lws_callback_reasons reason,
                                       void *user, void *in, size_t len) {
  switch (reason) {
  case LWS_CALLBACK_HTTP: {
    std::stringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << strlen(html_content) << "\r\n";
    response
        << "Connection: close\r\n"; // Important: Explicitly close the connection
    response << "\r\n";
    response << html_content;

    std::string response_str = response.str();
    lws_write(wsi, (unsigned char *)response_str.c_str(), response_str.length(),
              LWS_WRITE_HTTP_FINAL);

    return -1;
  }
  default:
    break;
  }
  return 0;
}