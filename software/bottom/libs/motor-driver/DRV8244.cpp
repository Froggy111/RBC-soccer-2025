#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "spdlog/spdlog.h"
#include "pinmap.hpp"
#include "pins.hpp"

class MotorDriver {
	PinInputControl inputControl;
	PinOutputControl outputControl;

	void initSPI(int32_t SPI_SPEED) {
		// Initialize SPI pins
		gpio_set_function(PinMap::SCK, GPIO_FUNC_SPI);
		gpio_set_function(PinMap::MOSI, GPIO_FUNC_SPI);
		gpio_set_function(PinMap::MISO, GPIO_FUNC_SPI);
		gpio_set_function(PinMap::CS, GPIO_FUNC_SPI);

		// Set SPI format
		spi_init(spi0, SPI_SPEED);
		spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

		// Set CS pin as output
		gpio_init(PinMap::CS);
		gpio_set_dir(PinMap::CS, GPIO_OUT);
		gpio_put(PinMap::CS, 1); // Set CS high (inactive)
	}

	void initPins() {
		// init NSLEEP
		gpio_init(PinMap::NSLEEP);
		gpio_set_dir(PinMap::NSLEEP, GPIO_OUT);
		gpio_put(PinMap::NSLEEP, 1); // Set NSLEEP high (active)

		// init NFAULT
	}

	void init(int32_t SPI_SPEED) {
        spdlog::info("---> Initializing DRV8244");

		spdlog::info("Initializing SPI");
        initSPI(SPI_SPEED);




		spdlog::info("DRV8244 initialized");
    }
};
