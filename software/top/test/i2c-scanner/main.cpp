#include "comms.hpp"
#include "types.hpp"
#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <pico/binary_info.h>
#include "pinmap.hpp"
#include "task.h"

using namespace types;

const u8 LED_PIN = 25;

bool reserved_addr(uint8_t addr) {
  return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void i2c_scan_task(void *args) {
  usb::CDC *cdc = (usb::CDC *)args;
  cdc->wait_for_CDC_connection(0xFFFFFFFF);

  i2c_init(i2c0, 100 * 1000);
  gpio_set_function((uint)pinmap::Pico::I2C0_SDA, GPIO_FUNC_I2C);
  gpio_set_function((uint)pinmap::Pico::I2C0_SCL, GPIO_FUNC_I2C);
  gpio_pull_up((uint)pinmap::Pico::I2C0_SDA);
  gpio_pull_up((uint)pinmap::Pico::I2C0_SCL);

  // Make the I2C pins available to picotool
  bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN,
                             GPIO_FUNC_I2C));

  comms::USB_CDC.printf("\r\nI2C Bus Scan\r\n");

  for (int addr = 0; addr < (1 << 7); ++addr) {
    // Perform a 1-byte dummy read from the probe address. If a slave
    // acknowledges this address, the function returns the number of bytes
    // transferred. If the address byte is ignored, the function returns
    // -1.

    // Skip over any reserved addresses.
    int ret;
    uint8_t rxdata;
    if (reserved_addr(addr))
      ret = PICO_ERROR_GENERIC;
    else
      ret = i2c_read_blocking(i2c0, addr, &rxdata, 1, false);

    if (ret >= 0) {
      comms::USB_CDC.printf("FOUND:%d\r\n", addr);
    } else {
      comms::USB_CDC.printf("NOPE\r\n");
    }
  }
  comms::USB_CDC.printf("\nDone.\r\n");

  while (true) {
  }
}

int main() {
  // Initialize LED
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  // Initialize USB CDC
  usb::CDC cdc = usb::CDC();
  cdc.init();

  // Create scanner task
  xTaskCreate(i2c_scan_task, "i2c_scan_task", 1024, &cdc, 10, NULL);

  // Start the scheduler
  vTaskStartScheduler();

  return 0;
}