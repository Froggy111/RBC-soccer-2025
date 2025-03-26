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


    //Test angles:
    
    for(float i = 0; i < 2*PI; i += (5.0*PI/180.0)){
        std::tuple<float, float, float, float> motor_speeds = motion_controller.translate(i, 1);
        std::cout << i << std::endl;
        std::cout << std::get<0>(motor_speeds) << " " << std::get<1>(motor_speeds) << " " << std::get<2>(motor_speeds) << " " << std::get<3>(motor_speeds) << std::endl;
        std::cout << "--------------------------------" << std::endl;
    }


    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    while (elapsed_time.count() < 500){
        std::tuple<float, float, float, float> needed_values = motion_controller.pid_output(0.0, 0.0, 0.0, 0.5);
        end_time = std::chrono::high_resolution_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    }

    start_time = std::chrono::high_resolution_clock::now();
    while (elapsed_time.count() < 500){
        std::tuple<float, float, float, float> needed_values = motion_controller.pid_output(0.0, 0.0, PI, 0.5);
        end_time = std::chrono::high_resolution_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    }

    start_time = std::chrono::high_resolution_clock::now();
    while (elapsed_time.count() < 500){
        std::tuple<float, float, float, float> needed_values = motion_controller.pid_output(0.0, 0.0, -(PI*0.5), 0.5);
        end_time = std::chrono::high_resolution_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    }

    start_time = std::chrono::high_resolution_clock::now();
    while (elapsed_time.count() < 500){
        std::tuple<float, float, float, float> needed_values = motion_controller.pid_output(0.0, 0.0, PI*0.5, 0.5);
        end_time = std::chrono::high_resolution_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    }

    start_time = std::chrono::high_resolution_clock::now();
    while (elapsed_time.count() < 500){
        std::tuple<float, float, float, float> needed_values = motion_controller.pid_output(0.0, 0.0, PI*0.25, 0.5);
        end_time = std::chrono::high_resolution_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    }

    start_time = std::chrono::high_resolution_clock::now();
    while (elapsed_time.count() < 500){
        std::tuple<float, float, float, float> needed_values = motion_controller.pid_output(0.0, 0.0, -(PI*0.25), 0.5);
        end_time = std::chrono::high_resolution_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    }

    start_time = std::chrono::high_resolution_clock::now();
    while (elapsed_time.count() < 500){
        std::tuple<float, float, float, float> needed_values = motion_controller.pid_output(0.0, 0.0, PI*0.75, 0.5);
        end_time = std::chrono::high_resolution_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    }

    start_time = std::chrono::high_resolution_clock::now();
    while (elapsed_time.count() < 500){
        std::tuple<float, float, float, float> needed_values = motion_controller.pid_output(0.0, 0.0, -(PI*0.75), 0.5);
        end_time = std::chrono::high_resolution_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    }

    return 0;
}