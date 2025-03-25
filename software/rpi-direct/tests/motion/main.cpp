#include <cstdio>
#include <iostream>
#include <chrono>
#include <tuple>
#include "motion.hpp"

#define PI 3.14159265358979323846

int main(){

    auto start_time = std::chrono::high_resolution_clock::now();

    MotionController motion_controller;
    motion_controller.init(1.0, 0.0, 0.0);

    auto end_time = std::chrono::high_resolution_clock::now();

    for(float i = 0; i < 2*PI; i += (5.0*PI/180.0)){
        std::tuple<int, int, int, int> motor_speeds = motion_controller.translate(i, 1);
        std::cout << i << std::endl;
        std::cout << std::get<0>(motor_speeds) << " " << std::get<1>(motor_speeds) << " " << std::get<2>(motor_speeds) << " " << std::get<3>(motor_speeds) << std::endl;
        std::cout << "--------------------------------" << std::endl;
    }

    return 0;
}