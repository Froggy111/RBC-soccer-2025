#include "pico/stdlib.h"
#include <cstdint>
#include "pico/stdio.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include <pico/stdio.h>
#include "spdlog/spdlog.h"

#define led_pin 16
#define led_count 1
#define brightness 50

// SPI Protocol
#define SPI_ADDRESS_MASK 0x3F00 // Mask for SPI register address bits
#define SPI_ADDRESS_POS 8       // Position for SPI register address bits
#define SPI_DATA_MASK 0x00FF    // Mask for SPI register data bits
#define SPI_DATA_POS 0          // Position for SPI register data bits
#define SPI_RW_BIT_MASK 0x4000  // Mask for SPI register read write indication bit

// PINS
#define CS_PIN 21
#define NSLEEP_PIN 9
#define NFAULT_PIN 7
#define IPROPI_PIN 29
#define IN2_PIN 5
#define IN1_PIN 4
#define DRVOFF_PIN 6
#define MOSI_PIN 19
#define MISO_PIN 20
#define SCK_PIN 18
#define SW1_PIN 14
#define SW2_PIN 15
#define ACCEL_DELAY_MICROS 2000
#define ACCEL_DELAY_MILLIS 2

const uint16_t A_IPROPI = 4750; // VQFN-HR package
const uint32_t IPROPI_resistance = 680;
const uint32_t ipropi_moving_average_size = 16384; // change this if uw, if its too big it wont fit

class MotorDriver {
	void init() {
		spdlog::info("---> Initializing DRV8244");
		stdio_init_all();
		gpio_init(CS_PIN);
		gpio_set_dir(CS_PIN, GPIO_OUT);
		gpio_put(CS_PIN, 1);
		gpio_init(NSLEEP_PIN);
		gpio_set_dir(NSLEEP_PIN, GPIO_OUT);
		gpio_put(NSLEEP_PIN, 1);
		gpio_init(IN2_PIN);
		gpio_set_dir(IN2_PIN, GPIO_OUT);
		gpio_init(IN1_PIN);
		gpio_set_dir(IN1_PIN, GPIO_OUT);
		gpio_init(NFAULT_PIN);
		gpio_set_dir(NFAULT_PIN, GPIO_IN);
		gpio_init(DRVOFF_PIN);
		gpio_set_dir(DRVOFF_PIN, GPIO_OUT);
		gpio_put(DRVOFF_PIN, 0);
		gpio_init(SW1_PIN);
		gpio_set_dir(SW1_PIN, GPIO_IN);
		gpio_init(SW2_PIN);
		gpio_set_dir(SW2_PIN, GPIO_IN);
	}
};