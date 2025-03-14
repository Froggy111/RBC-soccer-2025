#include "TSSP4038.hpp"
#include "include/IR.hpp"
#include "types.hpp"
#include "stdio.h"
#include <cstdio>
#include "comms.hpp"
#include "pinmap.hpp"

void IRsensor::init(){
    comms::USB_CDC.printf("---> Initializing IR\r\n");
    gpio_init(pinmap::Pico::IR1);
    gpio_init(pinmap::Pico::IR2);
    gpio_init(pinmap::Pico::IR3);
    gpio_init(pinmap::Pico::IR4);
    gpio_init(pinmap::Pico::IR5);
    gpio_init(pinmap::Pico::IR6);
    gpio_init(pinmap::Pico::IR7);
    gpio_init(pinmap::Pico::IR8);
    gpio_init(pinmap::Pico::IR9);
    gpio_init(pinmap::Pico::IR10);
    gpio_init(pinmap::Pico::IR11);
    gpio_init(pinmap::Pico::IR12);
    gpio_init(pinmap::Pico::IR13);
    gpio_init(pinmap::Pico::IR14);
    gpio_init(pinmap::Pico::IR15);
    gpio_init(pinmap::Pico::IR16);
    gpio_init(pinmap::Pico::IR17);
    gpio_init(pinmap::Pico::IR18);
    gpio_init(pinmap::Pico::IR19);
    gpio_init(pinmap::Pico::IR20);
    gpio_init(pinmap::Pico::IR21);
    gpio_init(pinmap::Pico::IR22);
    gpio_init(pinmap::Pico::IR23);
    gpio_init(pinmap::Pico::IR24);


}



