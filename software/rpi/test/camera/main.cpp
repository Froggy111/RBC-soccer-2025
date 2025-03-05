#include <stdio.h>
#include "pico/stdlib.h"
#include "cam.hpp"

int main() {
    stdio_init_all();
    
    printf("Camera Test Starting\n");
    
    // Initialize camera
    Camera camera;
    
    // Simple monitoring loop
    while (true) {
        // Example: Get camera frame or status
        camera.update();
        
        // Example: Print some camera stats
        printf("Camera status: %d\n", camera.getStatus());
        
        // Delay for readability
        sleep_ms(1000);
    }
    
    return 0;
}