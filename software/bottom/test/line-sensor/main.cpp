#include "ALSPT19.hpp"
// #include "pin_selector.hpp"
// #include "pins/digital_pins.hpp"
extern "C" {
#include <pico/stdlib.h>
#include <pico/stdio_usb.h>
#include <hardware/spi.h>
#include <pico/stdio.h>
#include <stdio.h>
}
#define comm_pin 28
LineSensor linesensor;

int main() {
    linesensor.init(1, spi1);
    while(true) {
        linesensor.select_channel(0);
        linesensor.read_raw();
        linesensor.read_voltage();
        float voltage = linesensor.read_voltage();
    }
}





