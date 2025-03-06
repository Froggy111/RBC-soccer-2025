#include <stdio.h>
#include "camera.hpp"

int main() {
    stdio_init_all();
    
    printf("Camera Test Starting\n");
    Camera camera;
    
    // Simple monitoring loop
    while (true) {
        camera.update();
        printf("Camera status: %d\n", camera.getStatus());
        
        // Delay for readability
        sleep_ms(1000);
    }
    
    return 0;
}