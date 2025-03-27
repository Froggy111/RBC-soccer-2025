extern "C" {
#include <hardware/spi.h>
#include <pico/time.h>
#include <hardware/gpio.h>
#include <hardware/timer.h>
#include <pico/stdlib.h>
}
#include "PMW3360.hpp"
#include "types.hpp"
#include "pin_selector.hpp"
#include "srom_firmware.hpp"
#include "pinmap.hpp"
#include "comms.hpp"

#define DEFAULT_CS 1 // CS high by default

#define PRODUCT_ID 0x00
#define MOTION 0x02

// Movement data registers
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
#define CONFIG2_RESET 0x00

#define ANGLE_TUNE 0x11
#define FRAME_CAPTURE 0x12
#define SROM_ENABLE 0x13
#define RUN_DOWNSHIFT 0x14

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
#define SPI_DATA_MASK 0x00FF // Mask for SPI register data bits
#define SPI_DATA_POS 0       // Position for SPI register data bits
#define SPI_RW_BIT_MASK                                                        \
  0x4000 // Mask for SPI register read write indication bit

#define FAULT_SUMMARY_REG 0x01

namespace mouse {
bool MouseSensor::dmux_init[2] = {false, false};
MCP23S17 MouseSensor::dmux1;
MCP23S17 MouseSensor::dmux2;

bool MouseSensor::init(int id, spi_inst_t *spi_obj_touse) {
  pins.set_mouse_sensor_id(id);
  _id = id;

  // * init DMUX
  dmux1.init(1, spi0);
  dmux2.init(2, spi0);

  // * init SPI
  spi_obj = spi_obj_touse;
  // Initialize SPI pins (except CS)
  gpio_set_function(pins.get_pin(SCLK), GPIO_FUNC_SPI);
  gpio_set_function(pins.get_pin(MOSI), GPIO_FUNC_SPI);
  gpio_set_function(pins.get_pin(MISO), GPIO_FUNC_SPI);

  // Initialize CS pin as GPIO
  if (_id == 0) {
    gpio_init((uint)pinmap::Pico::MOUSE1_SCS);
    gpio_set_dir((uint)pinmap::Pico::MOUSE1_SCS, GPIO_OUT);
    gpio_put((uint)pinmap::Pico::MOUSE1_SCS, DEFAULT_CS);
  } else {
    gpio_init((uint)pinmap::Pico::MOUSE2_SCS);
    gpio_set_dir((uint)pinmap::Pico::MOUSE2_SCS, GPIO_OUT);
    gpio_put((uint)pinmap::Pico::MOUSE2_SCS, DEFAULT_CS);
  }

  // Set SPI format
  spi_set_format(spi_obj, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

  // * init the rest
  init_pins();

  if (!init_registers()) {
    comms::USB_CDC.printf("Failed to init registers\r\n");
    return false;
  }

  if (!init_srom()) {
    comms::USB_CDC.printf("Failed to init SROM\r\n");
    return false;
  }

  return true;
}

void MouseSensor::init_pins() {
  if (_id == 1) {
    dmux1.init_gpio((uint)pinmap::Mux1A::MOUSE1_MOT, true, false);
    dmux1.init_gpio((uint)pinmap::Mux1A::MOUSE1_RST, true, true);
    dmux1.write_gpio((uint)pinmap::Mux1A::MOUSE1_RST, true, false);
  } else {
    dmux2.init_gpio((uint)pinmap::Mux2A::MOUSE2_MOT, true, false);
    dmux2.init_gpio((uint)pinmap::Mux2A::MOUSE2_RST, true, true);
    dmux2.write_gpio((uint)pinmap::Mux2A::MOUSE2_RST, true, false);
  }
}

bool MouseSensor::init_registers() {
  gpio_put(
      (uint)(_id == 1 ? pinmap::Pico::MOUSE1_SCS : pinmap::Pico::MOUSE2_SCS),
      1);
  gpio_put(
      (uint)(_id == 1 ? pinmap::Pico::MOUSE1_SCS : pinmap::Pico::MOUSE2_SCS),
      0);

  write8(POWER_UP_RESET, 0x5A);

  sleep_ms(55);

  // read from registers 2 3 4 5 6
  int res;
  res = read8(0x02);
  comms::USB_CDC.printf("%d\r\n", res);
  sleep_us(160);
  res = read8(0x03);
  comms::USB_CDC.printf("%d\r\n", res);
  sleep_us(160);
  res = read8(0x04);
  comms::USB_CDC.printf("%d\r\n", res);
  sleep_us(160);
  res = read8(0x05);
  comms::USB_CDC.printf("%d\r\n", res);
  sleep_us(160);
  res = read8(0x06);
  comms::USB_CDC.printf("%d\r\n", res);
  return true;
}

bool MouseSensor::init_srom() {
  // enable Rest_En in the CONFIG2 register
  // ~ write8(CONFIG2, CONFIG2_RESET & 0b11011111);
  write8(CONFIG2, 0x00);

  // First SROM Reg Write
  write8(SROM_ENABLE, 0x1D);

  sleep_ms(10);

  // Next SROM Reg Write
  write8(SROM_ENABLE, 0x18);

  // ~ sleep_us(120); // wait cuz they said at least 120 microseconds

  // * Load the SROM firmware
  // Custom SP\I write
  uint8_t srom_address = 0x62 | 0x80;

  gpio_put(
      (uint)(_id == 1 ? pinmap::Pico::MOUSE1_SCS : pinmap::Pico::MOUSE2_SCS),
      0);

  spi_write_blocking(spi_obj, &srom_address, 1);
  for (int i = 0; i < firmware_length; i++) {
    sleep_us(15); // delay required between bytes
    uint8_t data = firmware_data[i];
    spi_write_blocking(spi_obj, &data, 1);
  }
  sleep_us(15);

  gpio_put(
      (uint)(_id == 1 ? pinmap::Pico::MOUSE1_SCS : pinmap::Pico::MOUSE2_SCS),
      1);

  // * read PRODUCT_ID register
  uint8_t product_id = read8(PRODUCT_ID);
  if (product_id != product_ID) {
    comms::USB_CDC.printf("Product ID Mismatch: %d, expected: %d\r\n", product_id,
                          product_ID);
    return false;
  }

  // * read SROM register
  uint8_t srom_id = read8(SROM_ID);
  if (srom_id != firmware_ID) {
    comms::USB_CDC.printf("SROM ID Mismatch: %d, expected: %d\r\n", srom_id,
                          firmware_ID);
    return false;
  }

  // * wired mouse to config 2
  write8(CONFIG2, CONFIG2_RESET);
  return true;
}

void MouseSensor::write8(uint8_t reg, uint8_t value) {
  types::u8 buffer[2] = {(types::u8)(reg | 0x80), value};

  gpio_put(
      (uint)(_id == 1 ? pinmap::Pico::MOUSE1_SCS : pinmap::Pico::MOUSE2_SCS),
      0);
  spi_write_blocking(spi_obj, buffer, 2);
  sleep_us(35);

  gpio_put(
      (uint)(_id == 1 ? pinmap::Pico::MOUSE1_SCS : pinmap::Pico::MOUSE2_SCS),
      1);
}

uint8_t MouseSensor::read8(uint8_t reg) {
  types::u8 buffer = (types::u8)(reg & 0x7F);
  types::u8 response;

  gpio_put(
      (uint)(_id == 1 ? pinmap::Pico::MOUSE1_SCS : pinmap::Pico::MOUSE2_SCS),
      0);
  spi_write_blocking(spi_obj, &buffer, 1);
  sleep_us(160);
  spi_read_blocking(spi_obj, 0x00, &response, 1);

  gpio_put(
      (uint)(_id == 1 ? pinmap::Pico::MOUSE1_SCS : pinmap::Pico::MOUSE2_SCS),
      1);
  return response;
}

// * read specific registers
// read_motion_burst return 12 bytes (description in datasheet).
// The data is written into the array whose pointer is passed into the function as a parameter
void MouseSensor::read_motion_burst() {
  uint8_t reg[1] = {MOTION_BURST};
  uint8_t temp_buffer[12];

  gpio_put(
      (uint)(_id == 1 ? pinmap::Pico::MOUSE1_SCS : pinmap::Pico::MOUSE2_SCS),
      0);

  // write, wait 35 us, read
  spi_write_blocking(spi_obj, reg, 1);
  sleep_us(35);
  spi_read_blocking(spi_obj, 0x00, temp_buffer, 12);

  // Copy to buffer (assuming motion_burst_buffer is uint8_t[])
  for (int i = 0; i < 12; i++) {
    motion_burst_buffer[i] = temp_buffer[i];
  }

  gpio_put(
      (uint)(_id == 1 ? pinmap::Pico::MOUSE1_SCS : pinmap::Pico::MOUSE2_SCS),
      1);
}

//Return X_Delta Values
types::i16 MouseSensor::read_X_motion() {
  types::u8 X_L = read8(
      DELTA_X_L); // Important: DELTA_X_L Must be read first before DELTA_X_H

  sleep_us(160); // 160 microseconds delay between reads

  types::u8 X_H = read8(DELTA_X_H);

  types::i16 X_Val = ((X_H << 8) | X_L);

  return X_Val;
}

types::i16 MouseSensor::read_Y_motion() {
  types::u8 Y_L = read8(
      DELTA_Y_L); // Important: DELTA_Y_L Must be read first before DELTA_Y_H

  sleep_us(160); // 160 microseconds delay between reads

  types::u8 Y_H = read8(DELTA_Y_H);

  types::i16 Y_Val = ((Y_H << 8) | Y_L);

  return Y_Val;
}

types::u8 MouseSensor::read_squal() {
  types::u8 squal = read8(SQUAL);
  return squal;
}
} // namespace mouse