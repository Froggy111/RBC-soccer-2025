#pragma once
#include <tuple>
#include <cstdint>


class MotionController{
    public:

    void init(float kp, float ki, float kd);

    float angle_to_motor_speed(float angle);

    //pid_output -> Given the current heading, target heading, target direction and speed
    //              return the motor speeds to reach the target direction
    std::tuple<float, float, float, float> pid_output(float current_heading, float target_heading, float target_direction, float speed);
    
    
    void reset_pid();
    void set_pid_constants(float Kp, float Ki, float Kd);

    //translate -> Move the bot in a specific bearing, 
    //             taking the front of the bot to be north
    std::tuple<float, float, float, float> translate(float direction, float speed);

    //move_heading -> Given the bot's current heading, move in a specific bearing while
    //                maintaining the bot's heading
    std::tuple<float, float, float, float> move_heading(float current_direction, float bearing, float speed);


    //normalize -> map a given angle to a range [-PI, PI] in radians
    float normalize_angle(float angle);

    float map_angle(float angle);

    private:
    //Constants we prolly need to tune
    float translation_Kp = 1.0;
    float translation_Ki = 0.0;
    float translation_Kd = 0.0;

    float rotation_Kp = 1.0;
    float rotation_Ki = 0.0;
    float rotation_Kd = 0.0;

    int error_buffer_index = 0;
    int translation_integral = 0;
    int rotation_integral = 0;
    int last_translation_error = 0;
    int last_rotation_error = 0;
};