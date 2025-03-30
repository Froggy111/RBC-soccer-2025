#include <cstdio>
#include <iostream>
#include <chrono>
#include <tuple>
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>
#include "pf.hpp"

int main() {

    std::random_device rd;
    std::mt19937 rng(rd());

    ParticleFilter pf(5000, -915.0, 915.0, -1215.0, 1215.0); // 100 particles in a 10x10 space
    auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    for(int i = 0; i < 1000; i++){
        pf.added_new_input = false;
        start_time = std::chrono::high_resolution_clock::now();
    
        
        
        //Mouse
        if(i%3 == 2){
            pf.update_mouse(std::make_tuple(3.0, -1.5));
        }

        //Imu
        if(i%5 == 4){
            pf.update_imu(0.0*(M_PI)/180);
        }

        //Camera
        if(i == 0) pf.update_camera(std::make_tuple(4.0-i*0.5, 5.5-i*1.0, M_PI/2));
        if(i%7 != 6){
            pf.update_camera(std::make_tuple(4.0-i*0.5, 5.5-i*1.0, M_PI/2));
            pf.resample_particles(rng);
        }
        //pf.resample_particles(rng);
        
        //Predict values
        
        pf.predict(2.0, 3.0, 0.025);
        
        

        

        std::cout << "Iteration: " << i << '\n';
        end_time = std::chrono::high_resolution_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        //pf.printParticles();
        Particle estimated_position = pf.estimate_position();
        std::cout << "Elapsed: " << elapsed_time.count() << " us" << '\n';
        std::cout << "Predicted Position: " << estimated_position.x << " " << estimated_position.y << " " << estimated_position.theta << '\n';
        std::cout << "Expected: " << 4.0-i*0.5<< " " << 5.5-i*1.0 << " " << pf.normalize_angle((M_PI/2)) << '\n';
    }
}