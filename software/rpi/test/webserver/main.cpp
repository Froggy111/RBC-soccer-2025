#include <gtest/gtest.h>
#include "webserver.hpp"
#include <thread>
#include <chrono>


int main() {
    // Create a WebServer instance
    WebServer server(9000, 60);  

    // Start the server
    server.start();

    // Wait for a short time to allow the server to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check if the server is running
    server.isRunning();

    int req;
    while (true);
}