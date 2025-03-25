#include <cstdint>
#include <cmath>
#include <algorithm>
#include <tuple>

#include "include/motion.hpp"
#include "types.hpp"
#include "comms.hpp"

#define PI 3.14159265358979323846


std::tuple<int, int, int, int> MotionController::pid_output(float current_heading, float target_heading, float target_direction, float speed){
    int rotation_error = target_heading - current_heading;
    
    //Update the integral sum, and derivate
    rotation_integral += rotation_error;
    int rotation_derivate = rotation_error - last_rotation_error;

    std::tuple<int, int, int, int> translation_motor_values = move_heading(target_direction, speed);

    //Calculate PID Value
    int rotation_pid = rotation_error * rotation_Kp + rotation_integral * rotation_Ki + rotation_derivate * rotation_Kd;

    //Update last error 
    last_rotation_error = rotation_error;

    int motor1 = max(-1, min(1, std::get<0>(translation_motor_values) - rotation_pid));
    int motor2 = max(-1, min(1, std::get<1>(translation_motor_values) - rotation_pid));
    int motor3 = max(-1, min(1, std::get<2>(translation_motor_values) - rotation_pid));
    int motor4 = max(-1, min(1, std::get<3>(translation_motor_values) - rotation_pid));

    return std::make_tuple(motor1, motor2, motor3, motor4);   
}

void MotionController::reset_pid(){
    rotation_integral = 0;
    last_rotation_error = 0;
}

std::tuple<int, int, int, int> MotionController::translate(float direction, float speed){
    //Normalize angle to a range [-PI, PI] in radians.
    direction = normalize_angle(direction);

    //1 is top left, 2 is bottom left, 3 is top right, 4 is bottom right
    float motor1 = 0.0, motor2 = 0.0, motor3 = 0.0, motor4 = 0.0;
    if(direction >= 0 and direction <= PI*0.5){ //000 to 090
        motor1 = -1;
        motor2 = -std::cos(direction*2);
        motor3 = std::cos(direction*2);
        motor4 = 1;   
    } else if (direction > PI*0.5){//090 to 180
        motor1 = -std::cos((direction-PI*0.5)*2);
        motor2 = 1;
        motor3 = -1;
        motor4 = std::cos((direction-PI*0.5)*2);
    } else if (direction <= 0 and direction > -(PI*2)){//-090 to 000 (270 to 000)
        motor1 = -std::cos(-(direction)*2);
        motor2 = -1;
        motor3 = -1;
        motor4 = std::cos(direction*2);
    } else {//-180 to -090 (180 to 270)
        motor1 = 1;
        motor2 = -std::cos((PI*0.5+direction)*2); 
        motor3 = std::cos((PI*0.5+direction)*2); 
        motor4 = -1;   
    }

    return std::make_tuple(motor1, motor2, motor3, motor4);
}

std::tuple<int, int, int, int> MotionController::move_heading(float current_direction, float bearing, float speed){
    int resultant_direction;

    //Normalise the angle
    current_direction = normalize_angle(current_direction);

    //calculate resultant direction
    resultant_direction = normalize_angle(bearing - current_direction);

    return translate(resultant_direction, speed);
}

float MotionController::normalize_angle(float angle){
    while angle > PI{
        angle -= 2*PI;
    } 
    while angle < -PI{
        angle += 2*PI;
    }
    return angle;
}
