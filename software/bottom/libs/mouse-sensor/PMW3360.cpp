#include <cstdint>
#include <pico/time.h>
extern "C" {
#include <hardware/spi.h>
//#include <chrono>
//#include <thread>
//#include <time.h>
#include <hardware/gpio.h>
#include <hardware/timer.h>
#include <pico/stdlib.h>
#include "comms.hpp"

//#include <cpu.h>
}
#include "PMW3360.hpp"
#include "types.hpp"
#include "pin_selector.hpp"
#include "faults.hpp"

#define DEFAULT_CS 1 // CS high by default

#define PRODUCT_ID 0x00
#define MOTION 0x02

//Movement data registers

#define DELTA_X_L 0x03
#define DELTA_X_H 0x04 //must be read after DELTA_X_L
#define DELTA_Y_L 0x05
#define DELTA_Y_H 0x06 //must be read after DELTA_Y_L

#define SQUAL 0x07
#define RAW_DATA_SUM 0x08
#define MAXIMUM_RAW_DATA 0x09
#define MINIMUM_RAW_DATA 0x0A
#define SHUTTER_LOWER 0x0B
#define SHUTTER_UPPER 0x0C

#define CONTROL                                                                \
  0x0D //programmable invert of XY register scheme (inverts axes basically)

#define CONFIG1 0x0F
#define CONFIG1_RESET 0x31

#define CONFIG2 0x10
#define CONFIG2_RESET 0x20

#define ANGLE_TUNE 0x11
#define FRAME_CAPTURE 0x12
#define SROM_ENABLE 0x13
#define RUN_DOWNSHIFT 0x14

#define RES1_RATE_LOWER 0x15
#define RES1_RATE_UPPER 0x16
#define RES1_DOWNSHIFT 0x17

#define RES2_RATE_LOWER 0x18
#define RES2_RATE_UPPER 0x19
#define RES2_DOWNSHIFT 0x1A

#define RES3_RATE_LOWER 0x1B
#define RES3_RATE_UPPER 0x1C

#define OBSERVATION 0x24
#define DATA_OUT_LOWER 0x25
#define DATA_OUT_UPPER 0x26
#define RAW_DATA_GRAB 0x29

#define SROM_ID 0x2A

#define MIN_SQ_RUN 0x2B
#define RAW_DATA_THRESHOLD 0x2C

#define CONFIG5 0x2F //change X axis resolution (CPI)
#define CONFIG5_RESET 0x31

#define POWER_UP_RESET 0x3A
#define SHUTDOWN 0x3B
#define INVERSE_PRODUCT_ID 0x3F

#define LIFTCUTOFF_TUNE3 0x41
#define ANGLE_SNAP 0x42
#define LIFTCUTOFF_TUNE1 0x4A

#define MOTION_BURST 0x50

#define LIFTCUTOFF_TUNE_TIMEOUT 0x58
#define LIFTCUTOFF_TUNE_MIN_LENGTH 0x5A

#define SROM_LOAD_BURST 0x62

#define LIFT_CONFIG 0x63 //Configure lift detection height threshold
#define RAW_DATA_BURST 0x64

#define LIFTCUTOFF_TUNE2 0x65

#define SPI_ADDRESS_MASK_WRITE                                                 \
  0x3F00 // Mask for writing SPI register address bits
#define SPI_ADDRESS_MASK_READ                                                  \
  0x7F00                     // Mask for reading SPI register address bits
#define SPI_ADDRESS_POS 8    // Position for SPI register address bits
#define SPI_DATA_MASK 0x00FF // Mask for SPI register data bits
#define SPI_DATA_POS 0       // Position for SPI register data bits
#define SPI_RW_BIT_MASK                                                        \
  0x4000 // Mask for SPI register read write indication bit

#define FAULT_SUMMARY_REG 0x01

// inline void sleep_120ns() {
//   asm volatile("nop\n"
//                "nop\n"
//                "nop\n"
//                "nop\n"
//                "nop\n"
//                "nop\n"
//                "nop\n"
//                "nop\n"
//                "nop\n"
//                "nop\n"
//                "nop\n"
//                "nop\n"
//                "nop\n"
//                "nop\n"
//                "nop\n");
// }

// inline void sleep_20ns() {
//   asm volatile("nop\n"
//                "nop\n"
//                "nop\n");
// }

void MouseSensor::init(int id, spi_inst_t *spi_obj_touse) {

  // Initialize SPI pins (except CS)
  pinSelector.set_mouse_sensor_id(1);
  gpio_set_function(pinSelector.get_pin(SCLK), GPIO_FUNC_SPI);
  gpio_set_function(pinSelector.get_pin(MOSI), GPIO_FUNC_SPI);
  gpio_set_function(pinSelector.get_pin(MISO), GPIO_FUNC_SPI);

  // Set SPI Object
  spi_obj = spi_obj_touse;
  // Initialize CS pin as GPIO
  inputControl.init_digital(pinSelector.get_pin(CS), DEFAULT_CS);

  // Set SPI format
  spi_set_format(spi_obj, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
  init_registers();
}

void MouseSensor::init_pins() {
  // Initialize pins
  // MOT
  outputControl.init_digital(pinSelector.get_pin(MOT));

  // RST
  inputControl.init_digital(pinSelector.get_pin(RST), true);
}

void MouseSensor::write8(uint8_t reg, uint8_t value, int8_t expected) {
  types::u8 buffer[2] = {(types::u8)(reg | 0x80), value};

  inputControl.write_digital(pinSelector.get_pin(CS), 0);
  //busy_wait_ns(120); // Sleep for 120 nanoseconds
  busy_wait_us(1);

  spi_write_blocking(spi_obj, buffer, 2);
  inputControl.write_digital(pinSelector.get_pin(CS), 1);
  //busy_wait_ns(120);
  //std::this_thread::sleep_for(std::chrono::nanoseconds(120));
  busy_wait_us(1);
}

uint8_t MouseSensor::read8(uint8_t reg) {
  types::u8 buffer[2] = {(types::u8)(reg & 0x7F), 0x00};
  types::u8 response = 0;

  inputControl.write_digital(pinSelector.get_pin(CS), 0);
  //busy_wait_ns(120); // Sleep for 120 nanoseconds
  //std::this_thread::sleep_for(std::chrono::nanoseconds(120));
  busy_wait_us(1);
  
  spi_write_blocking(spi_obj, buffer, 1);
  busy_wait_us(160); //atleast 160 us
  spi_read_blocking(spi_obj, 0x00, &response, 1);
  inputControl.write_digital(pinSelector.get_pin(CS), 1);
  //busy_wait_ns(120); // Sleep for 120 nanoseconds
  //std::this_thread::sleep_for(std::chrono::nanoseconds(120));
  busy_wait_us(1);

  return response;
}

//read_motion_burst return 12 bytes (description in datasheet)
//The data is written into the array whose pointer is passed into the function as a parameter
void MouseSensor::read_motion_burst() {
  uint8_t reg = MOTION_BURST;

  // Start burst mode - use 8-bit commands
  inputControl.write_digital(pinSelector.get_pin(CS), 0);
  spi_write_blocking(spi_obj, &reg, 1); // 8-bit write

  // T_SRAD delay (at least 35μs)
  busy_wait_us(35);

  // Read burst data (12 bytes total)
  uint8_t temp_buffer[12];
  spi_read_blocking(spi_obj, 0, temp_buffer, 12); // 8-bit reads

  // Copy to buffer (assuming motion_burst_buffer is uint16_t[])
  for (int i = 0; i < 12; i++) {
    motion_burst_buffer[i] = temp_buffer[i];
  }

  inputControl.write_digital(pinSelector.get_pin(CS), 1); // Release CS pin

  //busy_wait_ns(500); //Sleep for 500 nanoseconds
  //std::this_thread::sleep_for(std::chrono::nanoseconds(500));
//   for (int i = 0; i < 4; i++)
//     sleep_120ns(); //total 480
//   sleep_20ns();
  busy_wait_us(1);
  
}

//Return X_Delta Values
types::u16 MouseSensor::read_X_motion() {
  types::u8 X_L = read8(
      DELTA_X_L); //Important: DELTA_X_L Must be read first before DELTA_X_H
  types::u8 X_H = read8(DELTA_X_H);

  types::u16 X_Val = ((X_L << SPI_ADDRESS_POS) & X_H);

  return X_Val;
}

types::u16 MouseSensor::read_Y_motion() {
  types::u8 Y_L = read8(
      DELTA_Y_L); //Important: DELTA_Y_L Must be read first before DELTA_Y_H
  types::u8 Y_H = read8(DELTA_Y_H);

  types::u16 Y_Val = ((Y_L << SPI_ADDRESS_POS) & Y_H);

  return Y_Val;
}

types::u16 MouseSensor::read_squal() {
  types::u8 squal = read8(SQUAL);
  types::u16 num_features =
      (squal << 3); //Number of features = SQUAL Register value * 8 (2^3)

  return num_features;
}

//! reading specific registers
bool MouseSensor::init_registers() {
  // Step 1: Power-up reset
  write8(POWER_UP_RESET, 0x5A, 0);

  // Delay at least 50ms
  sleep_ms(50);

  // Step 2: Read motion register to clear it
  read8(MOTION);

  // Step 3: SROM download sequence
  // First disable REST mode and prepare for SROM download
  write8(CONFIG2, 0x00, 0);
  write8(SROM_ENABLE, 0x1D, 0);

  // Wait 10ms
  sleep_ms(10);

  write8(SROM_ENABLE, 0x18, 0);

  // Load the SROM firmware (this depends on the firmware - consult PMW3360 datasheet)
  // Usually involves writing multiple bytes to SROM_LOAD_BURST register

  // After preparing for SROM download (which you've already done)
  // Write to the SROM_LOAD_BURST register and send firmware bytes
  const uint16_t firmware_length = 4094; // Check exact length in datasheet
  uint8_t
      firmware_data[firmware_length]; // This would contain the firmware bytes

  // Fill firmware_data with the actual SROM bytes from the datasheet

  // Begin SROM load
  uint8_t burst_cmd = SROM_LOAD_BURST;
  inputControl.write_digital(pinSelector.get_pin(CS), 0);
  spi_write_blocking(spi_obj, &burst_cmd, 1);

  // Send all firmware bytes
  for (int i = 0; i < firmware_length; i++) {
    spi_write_blocking(spi_obj, &firmware_data[i], 1);
    // Short delay between bytes if required
    busy_wait_us(15); // Check datasheet for exact timing
  }
  inputControl.write_digital(pinSelector.get_pin(CS), 1);

  // Wait for the SROM to load
  sleep_ms(10);

  // Step 4: Configure sensor settings
  // Set CPI (counts per inch)
  write8(CONFIG5, 0x00, 0);
  write8(CONFIG1, CONFIG1_RESET, 0);

  //printf("Sensor initialization successful\n");
  return true;
}
