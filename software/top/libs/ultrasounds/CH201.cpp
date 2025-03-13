extern "C" {
#include <pico/stdlib.h>

#define LEN_THRESH 8 // whacky hack
#include <invn/soniclib/soniclib.h>
#include <invn/soniclib/sensor_fw/ch201/ch201_gprmt.h>
#include <invn/soniclib/ch_rangefinder.h>

#include "hardware/gpio.h"
}
#include "CH201.hpp"
#include "pinmap.hpp"
#include "comms.hpp"

#define CH201_RTC_PULSE_SEQUENCE_TIME_LENGTH 1 // in ms
#define CH201_COUNT 16
#define CH201_FREERUN_INTERVAL 1 // in ms

ch_group_t Ultrasound::ch201_group;

void Ultrasound::group_init() {
  if (ch_group_init(&ch201_group, 16, 1,
                    CH201_RTC_PULSE_SEQUENCE_TIME_LENGTH)) {
    comms::USB_CDC.printf("CH201 group init failed\n");
  }
}

void Ultrasound::group_start() {
  ch_group_start(&ch201_group);
  while (true) {
    // TODO: Reuse this
    // for (int i = 0; i < CH201_COUNT; i++)
    //   sensor_int_callback(&ch201_group, i);
    sensor_int_callback(&ch201_group, 1);
  }
}

void Ultrasound::init(int us_id) {
  id = us_id;

  //* PIN CONFIGURATION
  // Configure GPIO pins
  gpio_init((uint)pinmap::Pico::US_NRST);
  gpio_set_dir((uint)pinmap::Pico::US_NRST, GPIO_OUT);

  //* ULTRASOUND INITIALIZATION
  // initialize using soniclib
  ch_init(&ch201_sensor, &ch201_group, CH201_COUNT, ch201_gprmt_init);
  ch_set_init_firmware(&ch201_sensor, ch201_gprmt_init);

  // set to free running mode
  ch_set_freerun_interval(&ch201_sensor, CH201_FREERUN_INTERVAL);
  ch_set_mode(&ch201_sensor, CH_MODE_FREERUN);

  // set the interrupts
  // ch_io_int_callback_set(&ch201_group, sensor_int_callback);
}

void Ultrasound::sensor_int_callback(ch_group_t *grp_ptr, uint8_t io_index) {
  // comms::USB_CDC.printf("CALLBACK: %d\r\n", io_index);
  ch_dev_t *dev_ptr = ch_get_dev_ptr(grp_ptr, io_index);
  if (dev_ptr != NULL) {
    int distance = ch_get_range(dev_ptr, CH_RANGE_ECHO_ONE_WAY);
    // comms::USB_CDC.printf("Distance: %d mm\r\n", distance);
  } else {
    // comms::USB_CDC.printf("Error: Could not get reading as dev_ptr is NULL\r\n");
  }
}