#pragma once
#include <tuple>
#include <cstdint>
#include <queue>

typedef std::tuple<float, float, float, float> tuple_4;
typedef std::tuple<float, float> tuple_2;

class MotionController{
    public:

    //TODO: Implement a queue system: For each position, move the bot to that position.
    //      When within a certain distance from point, go to next one
    //      If end of queue, then get the bot to a stop (and maintain it)
    //      Implement a function that can override it at any point in the queue????

    std::queue<std::tuple<float, float> > position_queue;
    std::tuple<float, float> last_position;


    void init(float kp, float ki, float kd);

    //angle_to_motor_speed -> Given an angle, return the motor speeds to reach that angle
    //                        using the formula (tan(theta) - 1)/(1 + tan(theta))
    float angle_to_motor_speed(float angle);

    //pid_output -> Given the current heading, target heading, target direction and speed
    //              return the motor speeds to reach the target direction
    std::tuple<float, float, float, float> pid_output(float current_heading, float target_heading, float target_direction, float speed);
    

    //position_pid -> Give the current position, target position, speed, do PID on it (using the pid_output function)

    std::tuple<float, float, float, float> position_pid(std::tuple<float, float> current_position, std::tuple<float, float> target_position, float current_heading, float target_heading,  float speed);



    //reset_pid -> Reset all integrals, previous errors to 0
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
    //map_angle -> map a angle to [-1, 1]
    float map_angle(float angle);



    private:
    //Constants we prolly need to tune

    //Probably useless for now
    float translation_Kp = 1.0;
    float translation_Ki = 0.0;
    float translation_Kd = 0.0;

    //Rotation PID Values, used for heading adjustment
    float rotation_Kp = 1.0;
    float rotation_Ki = 0.0;
    float rotation_Kd = 0.0;

    //
    int error_buffer_index = 0;
    int translation_integral = 0;
    int rotation_integral = 0;
    int last_translation_error = 0;
    int last_rotation_error = 0;
};