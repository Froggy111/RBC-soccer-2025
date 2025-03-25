#include <cstdint>
#include <cmath>
#include <algorithm>
#include <tuple>
#include <iostream>

#include "include/motion.hpp"



#define PI 3.14159265358979323846


float MotionController::angle_to_motor_speed(float angle){
    return (std::tan(angle) - 1)/(1 + std::tan(angle));
}

void MotionController::init(float kp, float ki, float kd){
    
    error_buffer_index = 0;
    translation_integral = 0;
    rotation_integral = 0;
    last_translation_error = 0;
    last_rotation_error = 0;
    
    rotation_Kp = kp;
    rotation_Ki = ki;
    rotation_Kd = kd;
    
}
std::tuple<float, float, float, float> MotionController::pid_output(float current_heading, float target_heading, float target_direction, float speed){
    int rotation_error = target_heading - current_heading;
    
    //Update the integral sum, and derivate
    rotation_integral += rotation_error;
    int rotation_derivate = rotation_error - last_rotation_error;

    std::tuple<float, float, float, float> translation_motor_values = move_heading(current_heading, target_direction, speed);

    //Calculate PID Value
    int rotation_pid = rotation_error * rotation_Kp + rotation_integral * rotation_Ki + rotation_derivate * rotation_Kd;

    //Update last error 
    last_rotation_error = rotation_error;

    int motor1 = std::max(float(-1.0), std::min(float(1.0), float(std::get<0>(translation_motor_values) - rotation_pid)));
    int motor2 = std::max(float(-1.0), std::min(float(1.0), float(std::get<1>(translation_motor_values) - rotation_pid)));
    int motor3 = std::max(float(-1.0), std::min(float(1.0), float(std::get<2>(translation_motor_values) - rotation_pid)));
    int motor4 = std::max(float(-1.0), std::min(float(1.0), float(std::get<3>(translation_motor_values) - rotation_pid)));

    return std::make_tuple(motor1, motor2, motor3, motor4);   
}

void MotionController::reset_pid(){
    rotation_integral = 0;
    last_rotation_error = 0;
}

std::tuple<float, float, float, float> MotionController::translate(float direction, float speed){
    //Normalize angle to a range [-PI, PI] in radians.
    direction = normalize_angle(direction);
    //std::cout << "Normalised Direction: " << (direction*180.0/PI) << std::endl;

    //1 is top left, 2 is bottom left, 3 is top right, 4 is bottom right
    float motor1 = 0.0, motor2 = 0.0, motor3 = 0.0, motor4 = 0.0;
    if(direction >= 0.0 and direction <= PI*0.5){ //000 to 090
        motor1 = -1.0;
        motor2 = -angle_to_motor_speed(direction);
        motor3 = angle_to_motor_speed(direction);
        //std::cout << "Motor 2: " << direction*2 << " " << -std::cos(direction*2) << std::endl;
        //std::cout << "Motor 3: " << direction*2 << " " << std::cos(direction*2) << std::endl;
        motor4 = 1.0;   
    } else if (direction > PI*0.5){//090 to 180
        motor1 = -angle_to_motor_speed((direction-PI*0.5));
        motor2 = 1.0;
        motor3 = -1.0;
        motor4 = angle_to_motor_speed((direction-PI*0.5));
    } else if (direction <= 0 and direction > -(PI*2)){//-090 to 000 (270 to 000)
        motor1 = -angle_to_motor_speed(-(direction));
        motor2 = -1.0;
        motor3 = -1.0;
        motor4 = angle_to_motor_speed(-(direction));
    } else {//-180 to -090 (180 to 270)
        motor1 = 1.0;
        motor2 = -angle_to_motor_speed((PI*0.5+direction)); 
        motor3 = angle_to_motor_speed((PI*0.5+direction)); 
        motor4 = -1.0;   
    }

    return std::make_tuple(motor1, motor2, motor3, motor4);
}

std::tuple<float, float, float, float> MotionController::move_heading(float current_direction, float bearing, float speed){
    int resultant_direction;

    //Normalise the angle
    current_direction = normalize_angle(current_direction);

    //calculate resultant direction
    resultant_direction = normalize_angle(bearing - current_direction);

    return translate(resultant_direction, speed);
}

float MotionController::normalize_angle(float angle){
    while(angle > PI){
        angle -= 2*PI;
    } 
    while(angle < -PI){
        angle += 2*PI;
    }
    return angle;
}

float MotionController::map_angle(float angle){
    angle = normalize_angle(angle);
    float mapped_angle = angle/(PI*0.25) - 1;
    mapped_angle = -mapped_angle;
    return mapped_angle;
}