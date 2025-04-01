#include "include/pf_test.hpp"
#include <complex>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <tuple>
#include <vector>
#include <random>

ParticleFilter::ParticleFilter(int num_particles, double x_min, double x_max, double y_min, double y_max) {
    std::uniform_real_distribution<double> dist_x(x_min, x_max);
    std::uniform_real_distribution<double> dist_y(y_min, y_max);
    std::uniform_real_distribution<double> dist_theta(-M_PI, M_PI);

    for (int i = 0; i < num_particles; ++i) {
        particles.push_back({dist_x(gen), dist_y(gen), dist_theta(gen), 1.0 / num_particles});
    }
}

double ParticleFilter::normalize_angle(double angle){
    while(angle > M_PI){
        angle -= 2*M_PI;
    } 
    while(angle < -M_PI){
        angle += 2*M_PI;
    }
    return angle;
}

void ParticleFilter::predict(double std_dev_pos, double std_dev_camera, double std_dev_theta) {
    std::normal_distribution<double> dist_x(0, std_dev_pos);
    std::normal_distribution<double> dist_y(0, std_dev_pos);
    std::normal_distribution<double> dist_theta(0, std_dev_theta);

    data_size = theta_changes.size(); // this would work with any other vector too
    double position_weight = 0.6, orientation_weight = 0.4;

    // for(int i = 0; i < data_size; i++){
    //     std::cout << "DEBUG: " << i << '\n';
    //     std::cout << "Camera Position and Theta: " << std::get<0>(position_archive[i]) << " " << std::get<1>(position_archive[i]) << " " << std::get<2>(position_archive[i]) << '\n';
    //     std::cout << "x_change and y_change: " << std::get<0>(position_changes[i]) + dist_x(gen) << " " << std::get<1>(position_changes[i]) + dist_y(gen) << '\n';
    //     std::cout << "theta change: " << theta_changes[i] << '\n';
    // }

    for(int i = 0; i < data_size; i++){
        double weight_sum = 0.0;
        double meas_x = std::get<0>(position_archive[i]), meas_y = std::get<1>(position_archive[i]), meas_theta = std::get<2>(position_archive[i]);
        for(auto& p: particles){
            double x_change = std::get<0>(position_changes[i]) + dist_x(gen);
            double y_change = std::get<1>(position_changes[i]) + dist_y(gen);
            //p.x += x_change;
            //p.y += y_change;
            
            p.x += (x_change)*(std::sin((p.theta + M_PI/2))) + (y_change)*(std::sin(p.theta));
            p.y += (x_change)*(std::cos((p.theta + M_PI/2))) + (y_change)*(std::cos(p.theta));
            p.theta += theta_changes[i];
            //std::cout << theta_changes[i] << '\n';
            if(meas_x != 10000){
                double error_x = meas_x - p.x;
                double error_y = meas_y - p.y;
                double error_dist = std::sqrt(error_x*error_x + error_y*error_y);
                double error_theta = normalize_angle(meas_theta - p.theta);
                error_theta = std::sqrt(error_theta*error_theta)*1800;
                //std::cout << error_x * error_x << " " << error_y * error_y << " " << error_theta*error_theta*423000 << '\n';

                // double pos_weight = exp(-error_dist * error_dist / (2 * std_dev_camera * std_dev_camera));
                // double angle_weight = exp((-error_theta * error_theta)/ (2 * std_dev_theta * std_dev_theta));
        
                // p.weight = position_weight*pos_weight + angle_weight*orientation_weight;
                double distance_sq = 0.6*(error_dist) + 0.4*(error_theta);
                p.weight = exp(-0.5 * distance_sq / (std_dev_camera * std_dev_camera));

                weight_sum += p.weight;
            }
        }

        if(meas_x != 10000){
            normalize_weights();
        }
    }
    theta_changes.clear();
    position_changes.clear();
    position_archive.clear();
    next_updated_index = 0;
}

void ParticleFilter::check_added_new_input(){
    if(not added_new_input){
        added_new_input = true;
        position_changes.push_back(std::make_tuple(0.0, 0.0));
        position_archive.push_back(std::make_tuple(10000, 10000, 10000));
        theta_changes.push_back(0.0);
    }
}

// void ParticleFilter::update_mouse(std::tuple<double, double> delta_position){
//     check_added_new_input();
//     added_velocity = true;
//     position_changes[position_changes.size() - 1]  = delta_position;
// }

void ParticleFilter::update_imu(double delta_vel_x, double delta_vel_y, double delta_theta){
    check_added_new_input();
    std::tuple<float, float> new_vel = std::make_tuple(std::get<0>(last_vel) + delta_vel_x, std::get<1>(last_vel) + delta_vel_y);
    int num_of_frames = position_changes.size() - next_updated_index + 1;
    int count = 1;
    for(int i = next_updated_index; i < position_changes.size(); i++){
        position_changes[i] = std::make_tuple((std::get<0>(last_vel) + delta_vel_x/num_of_frames*count), (std::get<1>(last_vel) + delta_vel_y/num_of_frames*count));
    }
    last_vel = position_changes[position_changes.size() - 1];
    next_updated_index = position_changes.size();

    theta_changes[theta_changes.size() - 1] = delta_theta;
}

void ParticleFilter::update_camera(std::tuple<double, double, double> new_position){

    check_added_new_input();

    position_archive[position_archive.size() - 1] = new_position;
}


void ParticleFilter::normalize_weights() {
    double sum_weights = 0.0;
    for (const auto& p : particles) {
        sum_weights += p.weight;
    }
    for (auto& p : particles) {
        p.weight /= sum_weights;
    }
}

// Function to compute entropy of particle weights
double ParticleFilter::calculate_entropy() {
    double entropy = 0.0;
    for (const auto& p : particles) {
        if (p.weight > 0) {
            entropy += p.weight * std::log2(p.weight);
        }
    }
    return -entropy;
}


// ------------------ LOW ENTROPY: CDF Resampling ------------------
std::vector<double> ParticleFilter::build_cdf() {
    std::vector<double> cdf(particles.size());
    cdf[0] = particles[0].weight;
    for (size_t i = 1; i < particles.size(); ++i) {
        cdf[i] = cdf[i - 1] + particles[i].weight;
    }
    return cdf;
}

int ParticleFilter::sample_cdf(const std::vector<double>& cdf, std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double rand_val = dist(rng);
    return lower_bound(cdf.begin(), cdf.end(), rand_val) - cdf.begin();
}

// ------------------ HIGH ENTROPY: Alias Method ------------------

void ParticleFilter::build_alias_table() {
    int n = particles.size();
    std::vector<double> scaled_probs(n);
    std::vector<int> small, large;

    for (int i = 0; i < n; ++i) {
        scaled_probs[i] = particles[i].weight * n;
        if (scaled_probs[i] < 1.0) small.push_back(i);
        else large.push_back(i);
    }

    prob.resize(n);
    alias.resize(n);

    while (!small.empty() && !large.empty()) {
        int small_idx = small.back(); small.pop_back();
        int large_idx = large.back();

        prob[small_idx] = scaled_probs[small_idx];
        alias[small_idx] = large_idx;

        scaled_probs[large_idx] -= (1.0 - scaled_probs[small_idx]);
        if (scaled_probs[large_idx] < 1.0) {
            small.push_back(large_idx);
            large.pop_back();
        }
    }

    while (!large.empty()) {
        prob[large.back()] = 1.0;
        large.pop_back();
    }
    while (!small.empty()) {
        prob[small.back()] = 1.0;
        small.pop_back();
    }
}

int ParticleFilter::sample_alias(std::mt19937& rng) {
    std::uniform_int_distribution<int> int_dist(0, prob.size() - 1);
    std::uniform_real_distribution<double> real_dist(0.0, 1.0);

    int idx = int_dist(rng);
    return (real_dist(rng) < prob[idx]) ? idx : alias[idx];
}

void ParticleFilter::resample_particles(std::mt19937& rng){
    int n = particles.size();
    normalize_weights();

    // Compute entropy
    double entropy = calculate_entropy();
    double threshold = log2(n);

    std::vector<Particle> new_particles(n);

    if (entropy < 0.5 * threshold) {
        // Low entropy → Use CDF-based binary search
        std::vector<double> cdf = build_cdf();
        std::cout << "Using CDF Resampling (Low Entropy)" << "Entropy: " << entropy << '\n';
        for (int i = 0; i < n; ++i) {
            int idx = sample_cdf(cdf, rng);
            new_particles[i] = particles[idx];
        }
    } else {
        // High entropy → Use Alias Method
        build_alias_table();
        std::cout << "Using Alias Method Resampling (High Entropy)" << "Entropy: " << entropy << '\n';
        for (int i = 0; i < n; ++i) {
            int idx = sample_alias(rng);
            new_particles[i] = particles[idx];
        }
    }

    // Reset weights after resampling
    for (auto& p : new_particles) {
        p.weight = 1.0 / n;
    }

    std::swap(particles, new_particles);
    
}

void ParticleFilter::printParticles() const {
    for (const auto& p : particles) {
        std::cout << "Particle at: (" << p.x << ", " << p.y << ") θ: " << p.theta 
                  << " Weight: " << p.weight << '\n';
    }
    std::cout << "----------------\n";
}

Particle ParticleFilter::estimate_position(){
    double x_est = 0.0, y_est = 0.0, sin_theta = 0.0, cos_theta = 0.0;

    for(const auto& p: particles){
        x_est += p.x * p.weight;
        y_est += p.y * p.weight;
        sin_theta += std::sin(p.theta)*p.weight;
        cos_theta +=  std::cos(p.theta)*p.weight;
    }

    double theta_est = normalize_angle(std::atan2(sin_theta, cos_theta));

    return {x_est, y_est, theta_est, 1.0};
}