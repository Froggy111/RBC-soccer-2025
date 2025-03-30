#include <cmath>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <queue>
#include <tuple>
#include "include/motion.hpp"
#include "comms.hpp"
#include "comms/identifiers.hpp"
#include "comms/usb.hpp"
#include "debug.hpp"

#define PI 3.14159265358979323846
#define SLIDING_WINDOW_SIZE 10

struct MotorRecvData {
    uint8_t id;
    uint16_t duty_cycle;
};

float MotionController::angle_to_motor_speed(float angle){
    return (std::tan(angle) - 1)/(1 + std::tan(angle));
}

void MotionController::init(float kp, float ki, float kd){
    
    rotation_integral = 0;
    last_rotation_error = 0;
    
    rotation_Kp = kp;
    rotation_Ki = ki;
    rotation_Kd = kd;
}

// Start a thread for continuous motion control
void MotionController::startControlThread() {
    if (controlThreadRunning.load()) {
        return;  // Thread already running
    }
    
    controlThreadRunning.store(true);
    controlThread = std::thread(&MotionController::controlThreadWorker, this);
}

// Stop the control thread
void MotionController::stopControlThread() {
    if (!controlThreadRunning.load()) {
        return;  // Thread not running
    }
    
    controlThreadRunning.store(false);
    
    if (controlThread.joinable()) {
        controlThread.join();
    }
}

// Worker function executed by the control thread
void MotionController::controlThreadWorker() {
    while (controlThreadRunning.load()) {
        debug::debug("Frame Count: %d", _processor.current_pos);

        Pos pos = _processor.current_pos;
        auto res = translate(std::make_tuple(0, 0.1f));

        // send motor values to the motors
        for (int i = 1; i <= 4; i++) {
            MotorRecvData motor_data = {
                .id         = (uint8_t) i,
                .duty_cycle = (uint16_t) (std::get<0>(res) * 12500)
            };

            comms::USB_CDC.write(
                usb::DeviceType::BOTTOM_PLATE,
                (types::u8)comms::SendBottomPicoIdentifiers::MOTOR_DRIVER_CMD,
                reinterpret_cast<uint8_t *>(&motor_data),
                sizeof(motor_data)
            );
        }
        
        // Prevent CPU thrashing with a small sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// Destructor - make sure to add this to your class implementation
MotionController::~MotionController() {
    stopControlThread();
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

std::tuple<float, float, float, float> MotionController::pid_output(float current_heading, float target_heading, float target_direction, float speed){
    int rotation_error = normalize_angle(target_heading - current_heading);
    
    //Update the rotation integral sum, and derivate
    rotation_integral += rotation_error;
    int rotation_derivate = rotation_error - last_rotation_error;

    //std::tuple<float, float, float, float> translation_motor_values = move_heading(current_heading, target_direction, speed);

    //Calculate rotation PID Value
    int rotation_pid = rotation_error * rotation_Kp + rotation_integral * rotation_Ki + rotation_derivate * rotation_Kd;

    //Update last rotation error 
    last_rotation_error = rotation_error;

    //std::tuple<float, float> position_change = std::make_tuple(get<0>(current_position)-get<0>(last_position), get<1>(current_position)-get<1>(last_position));

    //Update past x_vel and y_vel to update current velocities
    sum_x_velocities -= x_velocities.front();
    sum_y_velocities -= y_velocities.front();
    x_velocities.pop_front();
    y_velocities.pop_front();
    sum_x_velocities += std::get<0>(current_position)-std::get<0>(last_position);
    sum_y_velocities += std::get<1>(current_position)-std::get<1>(last_position);
    x_velocities.push_back(std::get<0>(current_position)-std::get<0>(last_position));
    y_velocities.push_back(std::get<1>(current_position)-std::get<1>(last_position));
    average_x_velocities = sum_x_velocities/SLIDING_WINDOW_SIZE;
    average_y_velocities = sum_y_velocities/SLIDING_WINDOW_SIZE;
    
    //Get current movement vector, target vector, error_change
    std::tuple<float, float> position_vector = form_vector(average_x_velocities*position_factor, average_y_velocities*position_factor);
    std::tuple<float, float> target_vector = std::make_tuple(target_direction-current_heading, speed);
    std::tuple<float, float> target_change = resolve_vector(target_vector);
    std::tuple<float, float> error_change = std::make_tuple(average_x_velocities-std::get<0>(target_change), average_y_velocities-std::get<1>(target_change));

    velocity_x_integral -= velocity_x_errors.front();
    velocity_x_errors.pop_front();
    velocity_y_integral -= velocity_y_errors.front();
    velocity_y_errors.pop_front();

    velocity_x_integral += std::get<0>(error_change);
    velocity_x_errors.push_back(std::get<0>(error_change));
    velocity_y_integral += std::get<1>(error_change);
    velocity_y_errors.push_back(std::get<1>(error_change));

    

    std::tuple<float, float> error_vector = form_vector(velocity_x_integral*velocity_Ki, velocity_y_integral*velocity_Ki);
    std::tuple<float, float> translation_vector = add_vectors(target_vector, error_vector);

    std::tuple<float, float, float, float> translation_motor_values = translate(translation_vector);

    int motor1 = float(std::get<0>(translation_motor_values));
    int motor2 = float(std::get<1>(translation_motor_values));
    int motor3 = float(std::get<2>(translation_motor_values));
    int motor4 = float(std::get<3>(translation_motor_values));

    motor1 = std::max(float(-1.0), std::min(float(1.0), float(motor1 - rotation_pid)));
    motor2 = std::max(float(-1.0), std::min(float(1.0), float(motor2 - rotation_pid)));
    motor3 = std::max(float(-1.0), std::min(float(1.0), float(motor3 - rotation_pid)));
    motor4 = std::max(float(-1.0), std::min(float(1.0), float(motor4 - rotation_pid)));

         
    calculate_expected_vel_dir(std::make_tuple(motor1, motor2, motor3, motor4));
    

    return std::make_tuple(motor1, motor2, motor3, motor4);   
}

std::tuple<float, float, float, float> MotionController::position_pid(std::tuple<float, float> current_position, std::tuple<float, float> target_position, float current_heading, float target_heading,  float speed){
    float delta_x = std::get<0>(target_position) - std::get<0>(current_position);
    float delta_y = std::get<1>(target_position) - std::get<1>(current_position);

    if(delta_x == 0) delta_x += 0.1;
    
    float target_direction = 0.0;
    target_direction = std::atan2(delta_y, delta_x);

    float distance_left = std::sqrt((delta_x*delta_x + delta_y*delta_y));
        
    if(position_queue.size() <= 1){
        speed = speed * (std::max(1.0, std::abs((100.0 - distance_left))/100.0));
    }

    if(not position_queue.empty()){
        if(distance_left < 7) position_queue.pop();
    }

    return pid_output(current_heading, target_heading, target_direction, speed);
}

void MotionController::reset_pid(){
    rotation_integral = 0;
    last_rotation_error = 0;
}

std::tuple<float, float> MotionController::resolve_vector(std::tuple<float, float> vec){
    float x_delta = 0.0, y_delta = 0.0;
    
    x_delta += std::get<1>(vec) * std::cos(-(std::get<0>(vec) - PI/2));
    y_delta += std::get<1>(vec) * std::sin(-(std::get<0>(vec) - PI/2));
    
    return std::make_tuple(x_delta, y_delta);
}

std::tuple<float, float> MotionController::add_vectors(std::tuple<float, float> vec1, std::tuple<float, float> vec2){
    float x_delta = 0.0, y_delta = 0.0;
    
    x_delta += std::get<1>(vec1) * std::cos(-(std::get<0>(vec1) - PI/2));
    y_delta += std::get<1>(vec1) * std::sin(-(std::get<0>(vec1) - PI/2));

    x_delta += std::get<1>(vec2) * std::cos(-(std::get<0>(vec2) - PI/2));
    y_delta += std::get<1>(vec2) * std::sin(-(std::get<0>(vec2) - PI/2));

    float resultant_direction = std::atan2(y_delta, x_delta);
    resultant_direction = normalize_angle(resultant_direction);

    float resultant_velocity = std::sqrt(x_delta*x_delta + y_delta*y_delta);

    return std::make_tuple(x_delta, y_delta);
}

std::tuple<float, float> MotionController::calculate_vel(std::tuple<float, float, float, float> motor_values){
    float x_delta = 0.0, y_delta = 0.0;

    //Resolve the motor speeds to get x and y delta values
    x_delta -= std::get<0>(motor_values) * std::cos(PI*0.25);
    y_delta -= std::get<0>(motor_values) * std::sin(PI*0.25);

    x_delta += std::get<1>(motor_values) * std::cos(PI*0.25);
    y_delta -= std::get<1>(motor_values) * std::sin(PI*0.25);

    x_delta -= std::get<2>(motor_values) * std::cos(PI*0.25);
    y_delta += std::get<2>(motor_values) * std::sin(PI*0.25);

    x_delta += std::get<3>(motor_values) * std::cos(PI*0.25);
    y_delta += std::get<3>(motor_values) * std::sin(PI*0.25);

    return std::make_tuple(x_delta, y_delta);
}

void MotionController::calculate_expected_vel_dir(std::tuple<float, float, float, float> motor_values){
    float x_delta = 0.0, y_delta = 0.0;

    //Resolve the motor speeds to get x and y delta values
    x_delta -= std::get<0>(motor_values) * std::cos(PI*0.25);
    y_delta -= std::get<0>(motor_values) * std::sin(PI*0.25);

    x_delta += std::get<1>(motor_values) * std::cos(PI*0.25);
    y_delta -= std::get<1>(motor_values) * std::sin(PI*0.25);

    x_delta -= std::get<2>(motor_values) * std::cos(PI*0.25);
    y_delta += std::get<2>(motor_values) * std::sin(PI*0.25);

    x_delta += std::get<3>(motor_values) * std::cos(PI*0.25);
    y_delta += std::get<3>(motor_values) * std::sin(PI*0.25);

    expected_direction = std::atan2(y_delta, x_delta);
    expected_direction = normalize_angle(expected_direction);

    expected_velocity = std::sqrt(x_delta*x_delta + y_delta*y_delta);
}

std::tuple<float, float> MotionController::form_vector(float x_error_vel, float y_error_vel){
    float error_dir = std::atan2(y_error_vel, x_error_vel);
    error_dir = normalize_angle(error_dir);

    float error_vel = std::sqrt(x_error_vel*x_error_vel + y_error_vel*y_error_vel);

    return std::tuple(error_dir, error_vel);
}

std::tuple<float, float, float, float> MotionController::translate(std::tuple<float, float> vec){
    float direction = std::get<0>(vec), speed = std::get<1>(vec);
    //Normalize angle to a range [-PI, PI] in radians.
    direction = normalize_angle(direction);
    //std::cout << "Normalised Direction: " << (direction*180.0/PI) << std::endl;

    //1 is top left, 2 is bottom left, 3 is top right, 4 is bottom right
    float motor1 = 0.0, motor2 = 0.0, motor3 = 0.0, motor4 = 0.0;
    if(direction >= 0.0 and direction <= PI*0.5){ //000 to 090
        //std::cout << "IF 1" << std::endl;
        motor1 = -1.0;
        motor2 = angle_to_motor_speed(direction);
        motor3 = -angle_to_motor_speed(direction);
        //std::cout << "Motor 2: " << direction*2 << " " << -std::cos(direction*2) << std::endl;
        //std::cout << "Motor 3: " << direction*2 << " " << std::cos(direction*2) << std::endl;
        motor4 = 1.0;   
    } else if (direction > PI*0.5){//090 to 180
        //std::cout << "IF 2" << std::endl;
        motor1 = angle_to_motor_speed((direction-PI*0.5));
        motor2 = 1.0;
        motor3 = -1.0;
        motor4 = -angle_to_motor_speed((direction-PI*0.5));
        
    } else if (direction <= 0 and direction >= -(PI*0.5)){//-090 to 000 (270 to 000)
        //std::cout << "IF 3" << std::endl;
        motor1 = -angle_to_motor_speed(PI*0.5 + (direction));
        motor2 = -1.0;
        motor3 = 1.0;
        motor4 = angle_to_motor_speed(PI*0.5 + (direction));
    } else {//-180 to -090 (180 to 270)
        //std::cout << "IF 4" << std::endl;
        motor1 = 1.0;
        motor2 = angle_to_motor_speed(std::abs(direction) - PI*0.5); 
        motor3 = -angle_to_motor_speed(std::abs(direction) - PI*0.5); 
        motor4 = -1.0;   
    }

    return std::make_tuple(speed*motor1, speed*motor2, speed*motor3, speed*motor4);
}

std::tuple<float, float, float, float> MotionController::move_heading(float current_direction, float bearing, float speed){
    int resultant_direction;

    //Normalise the angle
    current_direction = normalize_angle(current_direction);

    //calculate resultant direction
    resultant_direction = normalize_angle(bearing - current_direction);

    return translate(std::make_tuple(resultant_direction, speed));
}

float MotionController::map_angle(float angle){
    angle = normalize_angle(angle);
    float mapped_angle = angle/(PI*0.25) - 1;
    mapped_angle = -mapped_angle;
    return mapped_angle;
}

