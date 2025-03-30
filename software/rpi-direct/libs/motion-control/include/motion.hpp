#pragma once
#include "processor.hpp"
#include <deque>
#include <tuple>
#include <thread>
#include <atomic>

typedef std::tuple<float, float, float, float> tuple_4;
typedef std::tuple<float, float> tuple_2;

#define PI 3.14159265358979323846
#define VELOCITY_WINDOW_SIZE 10
#define ROTATION_ERRORS_WINDOW_SIZE 100
//using namespace std;

class MotionController {
  public:
    // Constructor (if not already defined)
    MotionController() = default;

    // Destructor to ensure thread cleanup
    ~MotionController();

    // Thread control methods
    void startControlThread();
    void stopControlThread();

    //TODO: Implement a queue system: For each position, move the bot to that position.
    //      When within a certain distance from point, go to next one
    //      If end of queue, then get the bot to a stop (and maintain it)
    //      Implement a expected velocity predictor

    std::queue<std::tuple<float, float> > position_queue;
    std::tuple<float, float> last_position;
    std::tuple<float, float> current_position;

    float expected_velocity = 0.0;
    float expected_direction = 0.0;

    

    void init(float rotation_kp, float rotation_ki, float rotation_kd, float velocity_kp, float velocity_ki, float velocity_kd);

    //normalize -> map a given angle to a range [-PI, PI] in radians
    float normalize_angle(float angle);

    //rotation_matrix -> given a vector in its x and y components, resolve it to axes rotated by <angle>
    std::tuple<float, float> rotation_matrix(std::tuple<float, float> vec, float angle);

    //map_angle -> map a angle to [-1, 1]
    float map_angle(float angle);

    //angle_to_motor_speed -> Given an angle, return the motor speeds to reach that angle
    //                        using the formula (tan(theta) - 1)/(1 + tan(theta))
    float angle_to_motor_speed(float angle);

    //pid_output -> Given the current heading, target heading, target direction and speed
    //              return the motor speeds to reach the target direction
    std::tuple<float, float, float, float> velocity_pid(float current_heading, float target_heading, float target_direction, float speed);
    

    //position_pid -> Give the current position, target position, speed, do PID on it (using the pid_output function)

    std::tuple<float, float, float, float> position_pid(std::tuple<float, float> current_position, std::tuple<float, float> target_position, float current_heading, float target_heading,  float speed);



    //reset_pid -> Reset all integrals, previous errors to 0
    void reset_pid();
    void set_pid_constants(float Kp, float Ki, float Kd);


    //add_vectors -> add 2 vectors (direction, speed), returns resultant 
    std::tuple<float, float> add_vectors(std::tuple<float, float> vec1, std::tuple<float, float> vec2);

    //calculate_expected_vel_dir -> Calculates expected velocity and direction given a tuple of motor values
    void calculate_expected_vel_dir(std::tuple<float, float, float, float> motor_values);

    //calculate_error_vel_dir -> Calculates error velocity and direction
    std::tuple<float, float> form_vector(float x_error_vel, float y_error_vel);

    //resolve_vector -> resolves vector (direction, speed) into x and y components
    std::tuple<float, float> resolve_vector(std::tuple<float, float> vec);

    //calculate_vel -> calculates x and y velocity
    std::tuple<float, float> calculate_vel(std::tuple<float, float, float, float> motor_values);

    //translate -> Move the bot in a specific bearing, 
    //             taking the front of the bot to be north
    std::tuple<float, float, float, float> translate(std::tuple<float, float> vec);

    //move_heading -> Given the bot's current heading, move in a specific bearing while
    //                maintaining the bot's heading
    std::tuple<float, float, float, float> move_heading(float current_direction, float bearing, float speed);

  private:
    // Thread control
    std::thread controlThread;
    std::atomic<bool> controlThreadRunning{false};
    camera::CamProcessor _processor;
    
    // Thread worker function
    void controlThreadWorker();
    
    //Velocity PID Values, used for controlling velocity
    float velocity_Kp = 2.0; //TODO: NEED TO TUNE
    float velocity_Ki = 1.0; //TODO: NEED TO TUNE
    float velocity_Kd = 0.0; //TODO: NEED TO TUNE

    //Rotation PID Values, used for heading adjustment
    float rotation_Kp = 0.16; //TODO: NEED TO TUNE
    float rotation_Ki = 0.08; //TODO: NEED TO TUNE
    float rotation_Kd = 0.0; //TODO: NEED TO TUNE

    //Sliding Window of errors in the x and y components of velocity
    std::deque<float> velocity_x_errors = std::deque<float>(50, 0.0);
    std::deque<float> velocity_y_errors = std::deque<float>(50, 0.0);

    //Sliding Window of errors in rotation
    std::deque<float> rotation_errors = std::deque<float>(ROTATION_ERRORS_WINDOW_SIZE, 0.0);

    //Sliding Window of masured velocities
    std::deque<float> x_velocities = std::deque<float>(VELOCITY_WINDOW_SIZE, 0.0);
    std::deque<float> y_velocities = std::deque<float>(VELOCITY_WINDOW_SIZE, 0.0);

    //Integrals (sum) for errors in x and y components of velocity
    float velocity_x_integral = 0.0;
    float velocity_y_integral = 0.0;

    //Sum of velocites (in x and y components)
    float sum_x_velocities = 0.0;
    float sum_y_velocities = 0.0;

    //Average of veocities (in x and y components)
    float average_x_velocities = 0.0;
    float average_y_velocities = 0.0;

    //Rotation integral and last error in rotation
    float rotation_integral = 0;
    float last_rotation_error = 0;


    //Factor to multiply position values by, so that at max speed, the change between last
    //and first is approx. 1
    float position_factor = 1.0; //TODO: NEED TO TUNE
};