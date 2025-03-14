#include "comms.hpp"
#include "types.hpp"
#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include "pinmap.hpp"
#include "task.h"

using namespace types;

const u8 LED_PIN = 25;

void i2c_scan_task(void *args) {
  usb::CDC *cdc = (usb::CDC *)args;
  cdc->wait_for_CDC_connection(0xFFFFFFFF);

  // Initialize I2C bus 0
  if (!i2c_init(i2c0, 100 * 1000)) {
    comms::USB_CDC.printf("I2C Initialization Failed!\r\n");
    return;
  } else {
    comms::USB_CDC.printf("I2C Initialization Successful!\r\n");
  }

  gpio_set_function((uint)pinmap::Pico::I2C0_SDA, GPIO_FUNC_I2C);
  gpio_set_function((uint)pinmap::Pico::I2C0_SCL, GPIO_FUNC_I2C);
  gpio_pull_up((uint)pinmap::Pico::I2C0_SDA);
  gpio_pull_up((uint)pinmap::Pico::I2C0_SCL);

  comms::USB_CDC.printf("I2C Bus Scanner Started\r\n");
  comms::USB_CDC.printf("Looking for devices at 0x45 and 0x29\r\n");

  int devices_found = 0;
  bool found_0x45 = false;
  bool found_0x29 = false;

  comms::USB_CDC.printf("\r\n--- I2C Bus Scan ---\r\n");

  // Scan all addresses
  for (int addr = 0; addr <= 0x7F; addr++) {
    uint8_t rxdata;
    // Use timeout version of read function (1000us = 1ms timeout)
    int ret = i2c_read_timeout_us(i2c0, addr, &rxdata, 1, false, 1000);
    comms::USB_CDC.printf("Finding 0x%02X: %d\r\n", addr, ret);

    if (ret >= 0) {
      // Device acknowledged address
      comms::USB_CDC.printf("Device found at address 0x%02X\r\n", addr);
      devices_found++;

      // Check for our specific addresses of interest
      if (addr == 0x45)
        found_0x45 = true;
      if (addr == 0x29)
        found_0x29 = true;
    }
  }

  // Summary
  comms::USB_CDC.printf("\r\nScan complete. Found %d devices.\r\n",
                        devices_found);
  comms::USB_CDC.printf("Address 0x45: %s\r\n",
                        found_0x45 ? "FOUND" : "NOT FOUND");
  comms::USB_CDC.printf("Address 0x29: %s\r\n",
                        found_0x29 ? "FOUND" : "NOT FOUND");
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