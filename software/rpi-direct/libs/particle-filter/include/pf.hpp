#pragma once
#include <iostream>
#include <random>
#include <vector>
#include <tuple>
#include <cmath>
#include <algorithm>

struct Particle {
    double x, y, theta;  // Position (x, y) and orientation theta
    double weight;
};

class ParticleFilter {
private:
    std::vector<Particle> particles;
    std::default_random_engine gen;
    int data_size = 0;
    double cumulative_theta = 0.0;
    std::vector<double> theta_changes;
    std::vector<std::tuple<double, double> > position_changes;
    std::vector<std::tuple<double, double, double> > position_archive; 
    
    
    double provisional_x = 0.0, provisional_y = 0.0;
    
public:
    std::vector<int> alias;
    std::vector<double> prob;
    bool added_new_input = false;
    int next_updated_index = 0;

    std::tuple<float, float> last_vel = std::make_tuple(0.0, 0.0);

    ParticleFilter(int num_particles, double x_min, double x_max, double y_min, double y_max);

    double normalize_angle(double angle);

    void predict(double std_dev_coords, double std_dev_camera, double std_dev_theta);

    void predict_old(double delta_x, double delta_y, double delta_theta, double std_dev);
       
    void update(double meas_x, double meas_y, double std_dev);

    void update_imu(double delta_vel_x, double delta_vel_y, double delta_theta);

    void update_camera(std::tuple<double, double, double> new_position);
    void resample();
    void check_added_new_input();

    void normalize_weights();

    std::vector<double> build_cdf();

    int sample_cdf(const std::vector<double>& cdf, std::mt19937& rng);

    void build_alias_table();

    int sample_alias(std::mt19937& rng);

    void resample_particles(std::mt19937& rng);

    double calculate_entropy();

    void printParticles() const;

    Particle estimate_position();
};




