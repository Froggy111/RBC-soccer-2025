#include "comms.hpp"
#include "types.hpp"

extern "C" {
#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <pico/stdlib.h>
#include <pico/binary_info.h>
#include <pico/time.h>
}

#include "pinmap.hpp"
#include "task.h"
#include "pins/MCP23S17.hpp"

using namespace types;

const u8 LED_PIN = 25;

MCP23S17 *dmux = nullptr;
TaskHandle_t i2c_scan_task_handle = nullptr;

bool reserved_addr(uint8_t addr) {
  return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void i2c_scan_task(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);

  // Initialize I2C bus 0
  if (!i2c_init(i2c0, 400 * 1000)) {
    comms::USB_CDC.printf("I2C Initialization Failed!\r\n");
    vTaskDelete(i2c_scan_task_handle);
  } else {
    comms::USB_CDC.printf("I2C Initialization Successful!\r\n");
  }

  // Initialize SPI bus 0
  if (!spi_init(spi0, 1000000)) {
    comms::USB_CDC.printf("SPI Initialization Failed!\r\n");
    vTaskDelete(i2c_scan_task_handle);
  } else {
    comms::USB_CDC.printf("SPI Initialization Successful!\r\n");
  }

  if (!dmux) {
    comms::USB_CDC.printf("dmux not allocated\r\n");
    vTaskDelete(i2c_scan_task_handle);
  } else {
    comms::USB_CDC.printf("dmux allocated");
  }

  // init dmux
  dmux->init(1, spi0);
  dmux->init_gpio(0, false, true);
  dmux->write_gpio(0, false, 1);
  dmux->init_gpio(1, false, true);
  dmux->write_gpio(1, false, 0);

  gpio_set_function((uint)pinmap::Pico::I2C0_SDA, GPIO_FUNC_I2C);
  gpio_set_function((uint)pinmap::Pico::I2C0_SCL, GPIO_FUNC_I2C);
  gpio_pull_up((uint)pinmap::Pico::I2C0_SDA);
  gpio_pull_up((uint)pinmap::Pico::I2C0_SCL);

  gpio_set_dir((uint)pinmap::Pico::US_NRST, GPIO_OUT);
  gpio_put((uint)pinmap::Pico::US_NRST, 0);

  sleep_ms(10);
  gpio_put((uint)pinmap::Pico::US_NRST, 1);

  // // Make the I2C pins available to picotool
  // bi_decl(bi_2pins_with_func((uint)pinmap::Pico::I2C0_SDA, (uint)pinmap::Pico::I2C0_SCL,
  //                            GPIO_FUNC_I2C));

  comms::USB_CDC.printf("\nI2C Bus Scan\r\n");

  while (true) {
    for (int addr = 0; addr < (1 << 7); ++addr) {
      // Perform a 1-byte dummy read from the probe address. If a slave
      // acknowledges this address, the function returns the number of bytes
      // transferred. If the address byte is ignored, the function returns
      // -1.

      // Skip over any reserved addresses.
      int ret;
      uint8_t rxdata = 0;
      if (reserved_addr(addr))
        ret = PICO_ERROR_GENERIC;
      else
        ret = i2c_read_blocking(i2c0, addr, &rxdata, 1, false);

      if (ret >= 0) {
        comms::USB_CDC.printf("FOUND:%d\r\n", addr);
      }
    }
  }
  comms::USB_CDC.printf("\nDone.\r\n");
}

int main() {
  // Initialize LED
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  gpio_set_function((uint)pinmap::Pico::SPI0_SCLK, GPIO_FUNC_SPI);
  gpio_set_function((uint)pinmap::Pico::SPI0_MOSI, GPIO_FUNC_SPI);
  gpio_set_function((uint)pinmap::Pico::SPI0_MISO, GPIO_FUNC_SPI);

  // Initialize USB CDC
  comms::USB_CDC.init();

  dmux = new MCP23S17();

  // Create scanner task
  xTaskCreate(i2c_scan_task, "i2c_scan_task", 1024, NULL, 10,
              &i2c_scan_task_handle);

  // Start the scheduler
  vTaskStartScheduler();

  return 0;
}