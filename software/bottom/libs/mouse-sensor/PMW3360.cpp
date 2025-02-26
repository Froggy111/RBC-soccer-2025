#include "PMW3360_ref.hpp"
#include <cstdint>
extern "C" {
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <stdio.h>
}
#include "PMW3360.hpp"
#include "types.hpp"
#include "pin_selector.hpp"

#define DEFAULT_CS 1     // CS high by default

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

#define CONTROL 0x0D //programmable invert of XY register scheme (inverts axes basically)

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



void MouseSensor::init_spi(types::u64 SPI_SPEED) {
    // Initialize SPI with error checking
    if (!spi_init(spi0, SPI_SPEED)) {
        printf("Error: SPI initialization failed\n");
        return;
    }

    // Initialize SPI pins (except CS)
    gpio_set_function(pinSelector.get_pin(SCK), GPIO_FUNC_SPI);
    gpio_set_function(pinSelector.get_pin(MOSI), GPIO_FUNC_SPI);
    gpio_set_function(pinSelector.get_pin(MISO), GPIO_FUNC_SPI);

    // Initialize CS pin as GPIO
    inputControl.init_digital(pinSelector.get_pin(CS), DEFAULT_CS);

    // Set SPI format
    spi_set_format(spi0, 16, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
}

void MouseSensor::init_pins() {
    // Initialize pins
    // MOT
    outputControl.init_digital(pinSelector.get_pin(MOT));
  
    // RST
    inputControl.init_digital(pinSelector.get_pin(RST));
}

bool MouseSensor::write8(uint8_t reg, uint8_t value, int8_t expected) {
    //* prepare data
    uint16_t reg_value = 0;
    uint16_t rx_data = 0;
    reg_value |= ((reg << SPI_ADDRESS_POS) &
                  SPI_ADDRESS_MASK_WRITE); // Add register address
    reg_value |= ((value << SPI_DATA_POS) & SPI_DATA_MASK); // Add data value
  
    //* Write & Read Feedback
    inputControl.write_digital(pinSelector.get_pin(CS), 0);
    int bytes_written =
        spi_write16_read16_blocking(spi0, &reg_value, &rx_data, 1);
    inputControl.write_digital(pinSelector.get_pin(CS), 1);
  
    printf("SPI Write - Sent: 0x%04X, Received: 0x%04X\n", reg_value, rx_data);
  
    //* Check for no errors in received bytes
    // First 2 MSBs bytes should be '1'
    if ((rx_data & 0xC000) != 0xC000) {
      printf("SPI Write - Error: Initial '1' MSB check bytes not found\n");
      return false;
    }
  
    // following 6 bytes are from fault summary
    if ((rx_data & 0x3F00) != 0x0000) {
      printf("SPI Write - Error: Fault summary bytes indicating error, %d\n",
             rx_data & 0x3F00);
      // get fault register
      types::u8 fault = read8(0x01);
  
      if (fault == 0) {
        printf("SPI Write - No fault found in fault register, moving on...\n");
      } else {
        printf("SPI Write - %s\n", Fault::get_fault_description(fault).c_str());
        return false;
      }
    }
  
    // Check remaining 8 bytes to match the sent data or expected return
    if (expected == -1) {
      if ((rx_data & 0x00FF) != value) {
        printf("SPI Write - Error: Data bytes do not match\n");
        return false;
      }
    } else {
      if ((rx_data & 0x00FF) != expected) {
        printf("SPI Write - Error: Data bytes do not match expected\n");
        return false;
      }
    }
  
    return bytes_written == 1;
  }

uint8_t MouseSensor::read8(uint8_t reg) {
    uint16_t reg_value = 0;
    uint16_t rx_data = 0;

    reg_value |= ((reg << SPI_ADDRESS_POS) & SPI_ADDRESS_MASK_READ);
    reg_value |= SPI_RW_BIT_MASK;

    inputControl.write_digital(pinSelector.get_pin(CS), 0);
    spi_write16_read16_blocking(spi0, &reg_value, &rx_data, 1);
    inputControl.write_digital(pinSelector.get_pin(CS), 1);

    printf("SPI Read - Sent: 0x%04X, Received: 0x%04X\n", reg_value, rx_data);

    //* Check for no errors in received bytes
    // First 2 MSBs bytes should be '1'
    if ((rx_data & 0xC000) != 0xC000) {
        printf("SPI Write - Error: Initial '1' MSB check bytes not found\n");
        return false;
    }

    // following 6 bytes are from fault summary
    if ((rx_data & 0x3F00) != 0x0000) {
        printf("SPI Write - Error: Fault summary bytes indicating error, %d\n",
                rx_data & 0x3F00);
        // get fault register
        types::u8 fault = read8(FAULT_SUMMARY_REG);

        if (fault == 0) {
        printf("SPI Write - No fault found in fault register, moving on...\n");
        } else {
        printf("SPI Write - %s\n", Fault::get_fault_description(fault).c_str());
        return false;
        }
    }

    return rx_data & 0xFF;
}


//read_motion_burst return 12 bytes (description in datasheet)
//The data is written into the array whose pointer is passed into the function as a parameter 
void MouseSensor::read_motion_burst(){
    uint8_t reg = MOTION_BURST;
    uint16_t reg_value = 0;

    reg_value |= ((reg << SPI_ADDRESS_POS) & SPI_ADDRESS_MASK_READ);
    reg_value |= SPI_RW_BIT_MASK;

    inputControl.write_digital(pinSelector.get_pin(CS), 0);
    spi_write16_read16_blocking(spi0, &reg_value, &mouse_sensor_buffer, 12);
    inputControl.write_digital(pinSelector.get_pin(CS), 1);

    //printf("SPI Read - Sent: 0x%04X, Received: 0x%04X\n", reg_value, rx_data);

}

//Return X_Delta Values
types::u16 MouseSensor::read_X_motion(){
    types::u8 X_L = read8(DELTA_X_L); //Important: DELTA_X_L Must be read first before DELTA_X_H
    types::u8 X_H = read8(DELTA_X_H);

    types::u16 X_Val = ((X_L << SPI_ADDRESS_POS) & X_H);

    return X_Val;
}

types::u16 MouseSensor::read_Y_motion(){
    types::u8 Y_L = read8(DELTA_Y_L); //Important: DELTA_Y_L Must be read first before DELTA_Y_H
    types::u8 Y_H = read8(DELTA_Y_H);

    types::u16 Y_Val = ((Y_L << SPI_ADDRESS_POS) & Y_H);

    return Y_Val;
}

types::u16 MouseSensor::read_squal(){
    types::u8 squal = read8(SQUAL);
    types::u16 num_features = (squal<<3); //Number of features = SQUAL Register value * 8 (2^3)

    return num_features;
}



//! reading specific registers
bool MouseSensor::init_registers() {
  
    //* CONFIG1 register
    if (!write8(CONFIG1, CONFIG1_RESET)) {
      printf("Error: Could not write to CONFIG1 register\n");
      return false;
    }
  
    //* CONFIG2 register
    if (!write8(CONFIG2, CONFIG2_RESET)) {
      printf("Error: Could not write to CONFIG2 register\n");
      return false;
    }
  
  
    //* CONFIG5 register
    if (!write8(CONFIG5, CONFIG5_RESET)) {
      printf("Error: Could not write to CONFIG5 register\n");
      return false;
    }


    return true;
}
  