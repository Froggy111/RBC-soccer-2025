#include "TSSP4038.hpp"
#include "include/IR.hpp"
#include "types.hpp"
#include "stdio.h"
#include <cstdio>

void IRsensor::init(){
    gpio_init(_pin)
    gpio_set_dir(_pin, GPIO_IN);
}

bool IRsensor::is_signal_detected(){
    return gpio_get(_pin) == 0;

}

void IRsensor::read_signal(){
    if (is_signal_detected()) {
    printf("Signal detected\n");
    } else {
        printf("Signal is not detected\n");
    }
}
