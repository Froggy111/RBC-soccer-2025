#include "include/ball_detect.hpp"
#include "IR.hpp"
//#include "comms.hpp"
//#include "debug.hpp"
#include "types.hpp"
#include <cmath>
#include <vector>
#include <cstdint>
#include <tuple>


namespace IR {

volatile ModulationData IR::modulation_buffer[SENSOR_COUNT] = {ModulationData()};

void IR::init(void) {
    //comms::USB_CDC.register_callback(
        //usb::DeviceType::MIDDLE_PLATE,
        //(types::u8)comms::RecvMiddlePicoIdentifiers::IR_DATA, data_processor);

    for(int i = 0; i < SENSOR_COUNT; i++){
        ir_headings[i] = ir_headings[i]*(M_PI)/180.0;
    }


}

void IR::data_processor(const types::u32 *data, int data_len) {
    for (int i = 0; i < SENSOR_COUNT; i++) modulation_data[i] = data[i];
    for (int i = 0; i < SENSOR_COUNT; i++) {
        debug::debug("IR sensor %u uptime: %u", i, modulation_data[i]);
        std::cout << "IR sensor "  << i << " uptime: " << modulation_data[i] << '\n';
        continue;
    }
    return;
}

void IR::map_data(){
    for(int i = 0; i < SENSOR_COUNT; i++){
        if(modulation_data[i] >= 200){
            modulation_data[i] = 200;
        } else if (modulation_data[i] >= 150){
            modulation_data[i] = 150;
        } else if (modulation_data[i] >= 100){
            modulation_data[i] = 100;
        } else {
            modulation_data[i] = 0;
        }
    }
}

float IR::normalize_angle(float angle){
    while(angle > M_PI){
        angle -= 2*M_PI;
    } 
    while(angle < -M_PI){
        angle += 2*M_PI;
    }
    return angle;
}

std::tuple<float, float> IR::find_ball(){
    std::vector<float> v;
    int closest_range = 0;
    float su = 0;
    for(int i = 0; i < SENSOR_COUNT; i++){
        if(modulation_data[i] > closest_range){
            v.clear();
            closest_range = modulation_data[i];
            v.push_back(ir_headings[i]);
        } else if (modulation_data[i] == closest_range){
            v.push_back(ir_headings[i]);
        }
    }

    for(int i = 0; i < v.size(); i++){
        if(i == 0){ 
            su += v[i];
            continue;
        }
        if(v[i] > v[i-1]){
            v[i] -= (M_PI)*2;
        }
        su += v[i];
        
    }

    float ball_heading = normalize_angle(su/v.size());
    float ball_dist = 0.0;
    switch(closest_range){
        case 0:
            ball_dist = 10000.0;
            break;
        case 100:
            ball_dist = 3.0;
            break;
        case 150:
            ball_dist = 2.0;
            break;
        case 200:
            ball_dist = 1.0;
            break;
        default:
            ball_dist = 0.5;
            break;
    }
    return std::make_tuple(ball_heading, ball_dist);

}



IR IR_sensors = IR();

} // namespace IR