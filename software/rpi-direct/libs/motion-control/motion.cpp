#include "include/motion.hpp"
#include "debug.hpp"
#include "motors.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <queue>
#include <tuple>

#define PI 3.14159265358979323846
#define SLIDING_WINDOW_SIZE 12

struct MotorRecvData {
    uint8_t id;
    uint16_t duty_cycle;
};

float MotionController::angle_to_motor_speed(float angle) {
    return (std::tan(angle) - 1) / (1 + std::tan(angle));
}

void MotionController::init(float rotation_kp, float rotation_ki,
                            float rotation_kd, float velocity_kp,
                            float velocity_ki, float velocity_kd) {

    rotation_integral   = 0;
    last_rotation_error = 0;

    rotation_Kp = rotation_kp;
    rotation_Ki = rotation_ki;
    rotation_Kd = rotation_kd;

    velocity_Kp = velocity_kp;
    velocity_Ki = velocity_ki;
    velocity_Kd = velocity_kd;
}

// Start a thread for continuous motion control
void MotionController::startControlThread() {
    if (controlThreadRunning.load()) {
        return; // Thread already running
    }

    controlThreadRunning.store(true);
    controlThread = std::thread(&MotionController::controlThreadWorker, this);
}

// Stop the control thread
void MotionController::stopControlThread() {
    if (!controlThreadRunning.load()) {
        return; // Thread not running
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

        Pos pos  = _processor.current_pos;
        auto res = translate(std::make_tuple(0, 0.1f));

        // send motor values to the motors
        motors::command_motor(1, std::get<0>(res));
        motors::command_motor(2, std::get<1>(res));
        motors::command_motor(3, std::get<2>(res));
        motors::command_motor(4, std::get<3>(res));

        // Prevent CPU thrashing with a small sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// Destructor - make sure to add this to your class implementation
MotionController::~MotionController() { stopControlThread(); }

void MotionController::update_position(std::tuple<float, float> ne_pos){
    
    if(current_position == std::make_tuple(-10000, -10000)){
        current_position = ne_pos;
        last_position = current_position;
    } else {
        last_position = current_position;
        current_position = ne_pos;
    }
}

float MotionController::normalize_angle(float angle) {
    while (angle > PI) {
        angle -= 2 * PI;
    }
    while (angle < -PI) {
        angle += 2 * PI;
    }
    return angle;
}

std::tuple<float, float>
MotionController::rotation_matrix(std::tuple<float, float> vec, float angle) {
    float new_x = 0.0;
    float new_y = 0.0;

    new_x +=
        std::get<0>(vec) * std::cos(angle) + std::get<1>(vec) * std::sin(angle);
    new_y += -std::get<0>(vec) * std::sin(angle) +
             std::get<1>(vec) * std::cos(angle);

    return std::make_tuple(new_x, new_y);
}

std::tuple<float, float, float, float>
MotionController::velocity_pid(float current_heading, float target_heading,
                               float target_direction, float speed) {

    //Normalise all input angles to [-PI, PI] for consistency
    current_heading = normalize_angle(current_heading);
    target_heading = normalize_angle(target_heading);
    target_direction = normalize_angle(target_direction);

    float rotation_error = normalize_angle(target_heading - current_heading);

    //Update the rotation integral sum, and derivate
    rotation_integral += rotation_error;
    rotation_integral -= rotation_errors.front();
    rotation_errors.pop_front();
    rotation_errors.push_back(rotation_error);

    float rotation_derivate = rotation_error - last_rotation_error;

    //std::tuple<float, float, float, float> translation_motor_values = move_heading(current_heading, target_direction, speed);

    //Calculate rotation PID Value
    float rotation_pid = rotation_error * rotation_Kp +
                       rotation_integral * rotation_Ki +
                       rotation_derivate * rotation_Kd;

    //Update last rotation error
    last_rotation_error = rotation_error;

    //std::tuple<float, float> position_change = std::make_tuple(get<0>(current_position)-get<0>(last_position), get<1>(current_position)-get<1>(last_position));

    //Update past x_vel and y_vel to update current velocities
    sum_x_velocities -= x_velocities.front();
    sum_y_velocities -= y_velocities.front();
    x_velocities.pop_front();
    y_velocities.pop_front();

    std::tuple<float, float> position_change = std::make_tuple(
        std::get<0>(current_position) - std::get<0>(last_position),
        std::get<1>(current_position) - std::get<1>(last_position));
    //position_change = rotation_matrix(position_change, current_heading);

    sum_x_velocities += std::get<0>(position_change);
    sum_y_velocities += std::get<1>(position_change);
    
    x_velocities.push_back(std::get<0>(position_change));
    y_velocities.push_back(std::get<1>(position_change));
    average_x_velocities = sum_x_velocities / VELOCITY_WINDOW_SIZE;
    average_y_velocities = sum_y_velocities / VELOCITY_WINDOW_SIZE;

    //Get current movement vector, target vector, error_change
    std::tuple<float, float> target_vector =
        std::make_tuple(target_direction - current_heading, speed);
    std::tuple<float, float> target_change = resolve_vector(target_vector);
    std::tuple<float, float> error_change =
        std::make_tuple(std::get<0>(target_change) - average_x_velocities,
                        std::get<1>(target_change) - average_y_velocities);

    velocity_x_change -= velocity_x_errors.front();
    velocity_x_errors.pop_front();
    velocity_y_change -= velocity_y_errors.front();
    velocity_y_errors.pop_front();

    velocity_x_integral += std::get<0>(error_change);
    velocity_x_change += std::get<0>(error_change);
    velocity_x_errors.push_back(std::get<0>(error_change));
    velocity_y_integral += std::get<1>(error_change);
    velocity_y_change += std::get<1>(error_change);
    velocity_y_errors.push_back(std::get<1>(error_change));

    // if(velocity_x_change < 1e-2 and velocity_y_change < 1e-2){
    //     velocity_x_integral = 0.0;
    //     velocity_y_integral = 0.0;
    //     velocity_x_change = 0.0;
    //     velocity_y_change = 0.0;
    //     std::fill(velocity_x_errors.begin(), velocity_x_errors.end(), 0.0);
    //     std::fill(velocity_y_errors.begin(), velocity_y_errors.end(), 0.0);
    // }

    std::tuple<float, float> error_vector = form_vector(
        velocity_x_integral*velocity_Ki , velocity_y_integral*velocity_Ki);
    std::tuple<float, float> p_vector = form_vector(std::get<0>(error_change)*velocity_Kp, std::get<1>(error_change)*velocity_Kp);
    std::tuple<float, float> translation_vector =
        add_vectors(target_vector, error_vector);

    translation_vector = std::make_tuple(normalize_angle(std::get<0>(translation_vector) - current_heading), std::get<1>(translation_vector));

    std::cout << "Target Vector: " << std::get<0>(target_vector) << " " << std::get<1>(target_vector) << '\n';
    std::cout << "Integrals:  " << velocity_x_integral << " " << velocity_y_integral << '\n';
    //std::cout << "Integrals: " << velocity_x_integral << " " << velocity_y_integral << '\n';
    std::cout << "Error Vector: " << std::get<0>(error_vector) << " " << std::get<1>(error_vector) << '\n';
    std::cout << "Resultant Vector: " << std::get<0>(translation_vector) << " " << std::get<1>(translation_vector) << '\n';

    std::tuple<float, float, float, float> translation_motor_values =
        translate(translation_vector);

    float motor1 = float(std::get<0>(translation_motor_values));
    float motor2 = float(std::get<1>(translation_motor_values));
    float motor3 = float(std::get<2>(translation_motor_values));
    float motor4 = float(std::get<3>(translation_motor_values));

    motor1 = std::max(float(-1.0),
                      std::min(float(1.0), float(motor1 - rotation_pid)));
    motor2 = std::max(float(-1.0),
                      std::min(float(1.0), float(motor2 - rotation_pid)));
    motor3 = std::max(float(-1.0),
                      std::min(float(1.0), float(motor3 - rotation_pid)));
    motor4 = std::max(float(-1.0),
                      std::min(float(1.0), float(motor4 - rotation_pid)));

    calculate_expected_vel_dir(std::make_tuple(motor1, motor2, motor3, motor4));

    return std::make_tuple(motor1, motor2, motor3, motor4);
}

std::tuple<float, float, float, float>
MotionController::position_pid(std::tuple<float, float> target_position,
                               float current_heading, float target_heading,
                               float speed) {
    bool usingQueuePosition = false;

    if (target_position == std::make_tuple(10000, 10000)) {
        usingQueuePosition = true;
        if (position_queue.empty()) {
            //Queue is empty, so no point running lmao
            return std::make_tuple(0.0, 0.0, 0.0, 0.0);
        }
        current_position = position_queue.front();
    }

    float delta_x =
        std::get<0>(target_position) - std::get<0>(current_position);
    float delta_y =
        std::get<1>(target_position) - std::get<1>(current_position);

    

    float target_direction = 0.0;
    target_direction       = std::atan2(delta_y, delta_x);
    target_direction = (-target_direction) + (M_PI/2);
    //std::cout << "Target DIrection " << target_direction << '\n';
    float distance_left = std::sqrt((delta_x * delta_x + delta_y * delta_y));

    if ((!usingQueuePosition) ||
        (usingQueuePosition && position_queue.size() <= 1)) {
        speed = std::min(double(speed), speed * (std::min(1.0, std::abs(distance_left / 10.0))));
    }

    
    if (usingQueuePosition && !position_queue.empty()) {
        if (distance_left < 30)
            position_queue.pop();
    }
    //std::cout << "Command: " << current_heading << " " << target_heading << " " << target_direction << " " << speed << '\n';
    return velocity_pid(current_heading, target_heading, target_direction,
                        speed);
}

void MotionController::reset_pid() {
    rotation_integral   = 0;
    last_rotation_error = 0;
}

std::tuple<float, float>
MotionController::resolve_vector(std::tuple<float, float> vec) {
    float x_delta = 0.0, y_delta = 0.0;

    x_delta += std::get<1>(vec) * std::cos(-(std::get<0>(vec) - PI / 2));
    y_delta += std::get<1>(vec) * std::sin(-(std::get<0>(vec) - PI / 2));

    return std::make_tuple(x_delta, y_delta);
}

std::tuple<float, float>
MotionController::add_vectors(std::tuple<float, float> vec1,
                              std::tuple<float, float> vec2) {
    float x_delta = 0.0, y_delta = 0.0;

    x_delta += std::get<1>(vec1) * std::cos(-(std::get<0>(vec1) - PI / 2));
    y_delta += std::get<1>(vec1) * std::sin(-(std::get<0>(vec1) - PI / 2));

    x_delta += std::get<1>(vec2) * std::cos(-(std::get<0>(vec2) - PI / 2));
    y_delta += std::get<1>(vec2) * std::sin(-(std::get<0>(vec2) - PI / 2));

    float resultant_direction = std::atan2(y_delta, x_delta);
    resultant_direction       = normalize_angle(resultant_direction);

    float resultant_velocity = std::sqrt(x_delta * x_delta + y_delta * y_delta);

    return std::make_tuple(x_delta, y_delta);
}

std::tuple<float, float> MotionController::calculate_vel(
    std::tuple<float, float, float, float> motor_values) {
    float x_delta = 0.0, y_delta = 0.0;

    //Resolve the motor speeds to get x and y delta values
    x_delta -= std::get<0>(motor_values) * std::cos(PI * 0.25);
    y_delta -= std::get<0>(motor_values) * std::sin(PI * 0.25);

    x_delta += std::get<1>(motor_values) * std::cos(PI * 0.25);
    y_delta -= std::get<1>(motor_values) * std::sin(PI * 0.25);

    x_delta -= std::get<2>(motor_values) * std::cos(PI * 0.25);
    y_delta += std::get<2>(motor_values) * std::sin(PI * 0.25);

    x_delta += std::get<3>(motor_values) * std::cos(PI * 0.25);
    y_delta += std::get<3>(motor_values) * std::sin(PI * 0.25);

    return std::make_tuple(x_delta, y_delta);
}

void MotionController::calculate_expected_vel_dir(
    std::tuple<float, float, float, float> motor_values) {
    float x_delta = 0.0, y_delta = 0.0;

    //Resolve the motor speeds to get x and y delta values
    x_delta -= std::get<0>(motor_values) * std::cos(PI * 0.25);
    y_delta -= std::get<0>(motor_values) * std::sin(PI * 0.25);

    x_delta += std::get<1>(motor_values) * std::cos(PI * 0.25);
    y_delta -= std::get<1>(motor_values) * std::sin(PI * 0.25);

    x_delta -= std::get<2>(motor_values) * std::cos(PI * 0.25);
    y_delta += std::get<2>(motor_values) * std::sin(PI * 0.25);

    x_delta += std::get<3>(motor_values) * std::cos(PI * 0.25);
    y_delta += std::get<3>(motor_values) * std::sin(PI * 0.25);

    expected_direction = std::atan2(y_delta, x_delta);
    expected_direction = normalize_angle(expected_direction);

    expected_velocity = std::sqrt(x_delta * x_delta + y_delta * y_delta);
}

std::tuple<float, float> MotionController::form_vector(float x_error_vel,
                                                       float y_error_vel) {
    float error_dir = std::atan2(y_error_vel, x_error_vel);
    //std::cout << y_error_vel << " nn " << x_error_vel << " nn " << error_dir << '\n';
    error_dir = (-error_dir) + (M_PI/2);
    error_dir       = normalize_angle(error_dir);

    float error_vel =
        std::sqrt(x_error_vel * x_error_vel + y_error_vel * y_error_vel);

    return std::tuple(error_dir, std::max(0.0f, std::min(1.0f, error_vel)));
}

std::tuple<float, float, float, float>
MotionController::translate(std::tuple<float, float> vec) {
    float direction = std::get<0>(vec), speed = std::get<1>(vec);
    //Normalize angle to a range [-PI, PI] in radians.
    direction = normalize_angle(direction);
    //std::cout << "Normalised Direction: " << (direction*180.0/PI) << std::endl;

    //1 is top left, 2 is bottom left, 3 is top right, 4 is bottom right
    float motor1 = 0.0, motor2 = 0.0, motor3 = 0.0, motor4 = 0.0;
    if (direction >= 0.0 and direction <= PI * 0.5) { //000 to 090
        //std::cout << "IF 1" << std::endl;
        motor1 = -1.0;
        motor2 = angle_to_motor_speed(direction);
        motor3 = -angle_to_motor_speed(direction);
        //std::cout << "Motor 2: " << direction*2 << " " << -std::cos(direction*2) << std::endl;
        //std::cout << "Motor 3: " << direction*2 << " " << std::cos(direction*2) << std::endl;
        motor4 = 1.0;
    } else if (direction > PI * 0.5) { //090 to 180
        //std::cout << "IF 2" << std::endl;
        motor1 = angle_to_motor_speed((direction - PI * 0.5));
        motor2 = 1.0;
        motor3 = -1.0;
        motor4 = -angle_to_motor_speed((direction - PI * 0.5));

    } else if (direction <= 0 and
               direction >= -(PI * 0.5)) { //-090 to 000 (270 to 000)
        //std::cout << "IF 3" << std::endl;
        motor1 = -angle_to_motor_speed(PI * 0.5 + (direction));
        motor2 = -1.0;
        motor3 = 1.0;
        motor4 = angle_to_motor_speed(PI * 0.5 + (direction));
    } else { //-180 to -090 (180 to 270)
        //std::cout << "IF 4" << std::endl;
        motor1 = 1.0;
        motor2 = angle_to_motor_speed(std::abs(direction) - PI * 0.5);
        motor3 = -angle_to_motor_speed(std::abs(direction) - PI * 0.5);
        motor4 = -1.0;
    }

    return std::make_tuple(speed * motor1, speed * motor2, speed * motor3,
                           speed * motor4);
}

std::tuple<float, float, float, float>
MotionController::move_heading(float current_direction, float bearing,
                               float speed) {
    float resultant_direction;

    //Normalise the angle
    current_direction = normalize_angle(current_direction);

    //calculate resultant direction
    resultant_direction = normalize_angle(bearing - current_direction);

    return translate(std::make_tuple(resultant_direction, speed));
}

float MotionController::map_angle(float angle) {
    angle              = normalize_angle(angle);
    float mapped_angle = angle / (PI * 0.25) - 1;
    mapped_angle       = -mapped_angle;
    return mapped_angle;
}