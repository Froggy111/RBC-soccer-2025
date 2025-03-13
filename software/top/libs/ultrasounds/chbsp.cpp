#include "pinmap.hpp"
#include "pins/MCP23S17.hpp"
#include <cstdint>
#include <hardware/gpio.h>

extern "C" {
#include <invn/soniclib/soniclib.h>
#include <invn/soniclib/chirp_bsp.h>
#include <hardware/i2c.h>
#include <pico/stdlib.h>
}

#include "comms.hpp"
#include "pin_selector.hpp"

// Global DMUX instances
MCP23S17 *dmux1 = new MCP23S17();
MCP23S17 *dmux2 = new MCP23S17();

extern "C" {
void chbsp_print_str(const char *str) { comms::USB_CDC.printf("%s", str); }

void chbsp_debug_toggle(uint8_t dbg_pin_num) {
  // NOT USED
}

// Init Pins
void chbsp_init() {
  // Initialize DMUX
  dmux1->init(1, spi0);
  dmux2->init(2, spi0);

  // Init all dmux pins to output
  for (int i = 0; i < 32; i++) {
    if (i < 16) {
      dmux1->init_gpio(i % 8, i < 8, 1);
    } else {
      dmux2->init_gpio(i % 8, i < 24, 1);
    }
  }
}

// Reset Functions (CH101/CH201)
void chbsp_reset_assert(void) { gpio_put((uint)pinmap::Pico::US_NRST, 0); }

void chbsp_reset_release(void) { gpio_put((uint)pinmap::Pico::US_NRST, 1); }

// PROG Pin Control (CH101/CH201)
void chbsp_program_enable(ch_dev_t *dev_ptr) {
  uint8_t id = ch_get_dev_num(dev_ptr);

  int dmux_id, dmux_gpio_pin_no;
  bool dmux_on_A;

  DMUX_SELECTOR::select_dmux_pin(DMUX_SELECTOR::PIN_TYPE::PROG, id, dmux_id,
                                 dmux_gpio_pin_no, dmux_on_A);
  if (dmux_id == 1) {
    dmux1->init_gpio(dmux_gpio_pin_no, dmux_on_A, 1);
    dmux1->write_gpio(dmux_gpio_pin_no, dmux_on_A, 1);
  } else if (dmux_id == 2) {
    dmux2->init_gpio(dmux_gpio_pin_no, dmux_on_A, 1);
    dmux2->write_gpio(dmux_gpio_pin_no, dmux_on_A, 1);
  }
}

void chbsp_program_disable(ch_dev_t *dev_ptr) {
  uint8_t id = ch_get_dev_num(dev_ptr);

  int dmux_id, dmux_gpio_pin_no;
  bool dmux_on_A;

  DMUX_SELECTOR::select_dmux_pin(DMUX_SELECTOR::PIN_TYPE::PROG, id, dmux_id,
                                 dmux_gpio_pin_no, dmux_on_A);
  if (dmux_id == 1) {
    dmux1->init_gpio(dmux_gpio_pin_no, dmux_on_A, 1);
    dmux1->write_gpio(dmux_gpio_pin_no, dmux_on_A, 0);
  } else if (dmux_id == 2) {
    dmux2->init_gpio(dmux_gpio_pin_no, dmux_on_A, 1);
    dmux2->write_gpio(dmux_gpio_pin_no, dmux_on_A, 0);
  }
}

void chbsp_set_int1_dir_out(ch_dev_t *dev_ptr) {
  uint8_t id = ch_get_dev_num(dev_ptr);

  int dmux_id, dmux_gpio_pin_no;
  bool dmux_on_A;

  DMUX_SELECTOR::select_dmux_pin(DMUX_SELECTOR::PIN_TYPE::INT, id, dmux_id,
                                 dmux_gpio_pin_no, dmux_on_A);

  if (dmux_id == 1) {
    dmux1->init_gpio(dmux_gpio_pin_no, dmux_on_A, 1);
  } else if (dmux_id == 2) {
    dmux2->init_gpio(dmux_gpio_pin_no, dmux_on_A, 1);
  }
}

void chbsp_group_set_int1_dir_out(ch_group_t *grp_ptr) {
  for (uint8_t i = 0; i < grp_ptr->num_ports; i++) {
    ch_dev_t *dev_ptr = ch_get_dev_ptr(grp_ptr, i);
    if (dev_ptr != NULL) {
      chbsp_set_int1_dir_out(dev_ptr);
    }
  }
}

void chbsp_int1_clear(ch_dev_t *dev_ptr) {
  uint8_t id = ch_get_dev_num(dev_ptr);

  int dmux_id, dmux_gpio_pin_no;
  bool dmux_on_A;

  DMUX_SELECTOR::select_dmux_pin(DMUX_SELECTOR::PIN_TYPE::INT, id, dmux_id,
                                 dmux_gpio_pin_no, dmux_on_A);

  if (dmux_id == 1) {
    dmux1->init_gpio(dmux_gpio_pin_no, dmux_on_A, 1);
    dmux1->write_gpio(dmux_gpio_pin_no, dmux_on_A, 0);
  } else if (dmux_id == 2) {
    dmux2->init_gpio(dmux_gpio_pin_no, dmux_on_A, 1);
    dmux2->write_gpio(dmux_gpio_pin_no, dmux_on_A, 0);
  }
}

void chbsp_group_int1_clear(ch_group_t *grp_ptr) {
  for (uint8_t i = 0; i < grp_ptr->num_ports; i++) {
    ch_dev_t *dev_ptr = ch_get_dev_ptr(grp_ptr, i);
    if (dev_ptr != NULL) {
      chbsp_int1_clear(dev_ptr);
    }
  }
}

void chbsp_int1_set(ch_dev_t *dev_ptr) {
  uint8_t id = ch_get_dev_num(dev_ptr);

  int dmux_id, dmux_gpio_pin_no;
  bool dmux_on_A;

  DMUX_SELECTOR::select_dmux_pin(DMUX_SELECTOR::PIN_TYPE::INT, id, dmux_id,
                                 dmux_gpio_pin_no, dmux_on_A);

  if (dmux_id == 1) {
    dmux1->init_gpio(dmux_gpio_pin_no, dmux_on_A, 1);
    dmux1->write_gpio(dmux_gpio_pin_no, dmux_on_A, 1);
  } else if (dmux_id == 2) {
    dmux2->init_gpio(dmux_gpio_pin_no, dmux_on_A, 1);
    dmux2->write_gpio(dmux_gpio_pin_no, dmux_on_A, 1);
  }
}

void chbsp_group_int1_set(ch_group_t *grp_ptr) {
  for (uint8_t i = 0; i < grp_ptr->num_ports; i++) {
    ch_dev_t *dev_ptr = ch_get_dev_ptr(grp_ptr, i);
    if (dev_ptr != NULL) {
      chbsp_int1_set(dev_ptr);
    }
  }
}

// I2C Initialization
int chbsp_i2c_init(void) {
  comms::USB_CDC.printf("-> Initializing I2C\r\n");

  // init I2C pins
  gpio_set_function((uint)pinmap::Pico::I2C0_SDA, GPIO_FUNC_I2C);
  gpio_set_function((uint)pinmap::Pico::I2C0_SCL, GPIO_FUNC_I2C);
  gpio_pull_up((uint)pinmap::Pico::I2C0_SDA);
  gpio_pull_up((uint)pinmap::Pico::I2C0_SCL);

  // Configure GPIO pins
  gpio_init((uint)pinmap::Pico::US_NRST);
  gpio_set_dir((uint)pinmap::Pico::US_NRST, GPIO_OUT);
  return 0;
}

// Delay Functions
void chbsp_delay_us(uint32_t us) { sleep_us(us); }

void chbsp_delay_ms(uint32_t ms) { sleep_ms(ms); }

// Timestamp
uint32_t chbsp_timestamp_ms(void) {
  return to_ms_since_boot(get_absolute_time());
}

// * I2C Functions
// I2C Write Function
int chbsp_i2c_write(ch_dev_t *dev_ptr, const uint8_t *data, uint16_t num_bytes) {
  uint8_t addr = ch_get_i2c_address(dev_ptr);
  comms::USB_CDC.printf("I2C Write: addr=0x%02X len=%d\r\n", addr, num_bytes);
  
  int result = i2c_write_blocking(i2c0, addr, data, num_bytes, false);
  if (result != num_bytes) {
    comms::USB_CDC.printf("I2C Write Failed: expected=%d actual=%d\r\n", num_bytes, result);
    return 1;
  }
  return 0;
}

// I2C Memory Write Function
int chbsp_i2c_mem_write(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data,
                        uint16_t num_bytes) {
  uint8_t buf[num_bytes + 2];
  buf[0] = (mem_addr >> 8) & 0xFF;
  buf[1] = mem_addr & 0xFF;
  memcpy(buf + 2, data, num_bytes);
  return chbsp_i2c_write(dev_ptr, buf, num_bytes + 2);
}

// I2C Read Function
int chbsp_i2c_read(ch_dev_t *dev_ptr, uint8_t *data, uint16_t num_bytes) {
  uint8_t addr = ch_get_i2c_address(dev_ptr);
  comms::USB_CDC.printf("I2C Read: addr=0x%02X len=%d\r\n", addr, num_bytes);
  
  int result = i2c_read_blocking(i2c0, addr, data, num_bytes, false);
  if (result != num_bytes) {
    comms::USB_CDC.printf("I2C Read Failed: expected=%d actual=%d\r\n", num_bytes, result);
    return 1;
  }
  return 0;
}

// I2C Memory Read Function
int chbsp_i2c_mem_read(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data,
                       uint16_t num_bytes) {
  uint8_t addr = ch_get_i2c_address(dev_ptr);
  uint8_t addr_buf[2] = {(uint8_t)((mem_addr >> 8) & 0xFF),
                         (uint8_t)(mem_addr & 0xFF)};

  // Write address bytes first
  if (i2c_write_blocking(i2c0, addr, addr_buf, 2, true) != 2) {
    return 1;
  }

  // Then read data
  return i2c_read_blocking(i2c0, addr, data, num_bytes, false) == num_bytes ? 0
                                                                            : 1;
}

// I2C Reset Function
void chbsp_i2c_reset(ch_dev_t *dev_ptr) {
  // The dev_ptr might be needed in more advanced implementations
  (void)dev_ptr; // Cast to void to avoid unused parameter warning
  
  // Reset the I2C bus by deinitializing and reinitializing
  i2c_deinit(i2c0);
  
  // Reinitialize with standard settings (400 kHz)
  i2c_init(i2c0, 400 * 1000);
  
  // Reset GPIO functions for I2C pins
  gpio_set_function((uint)pinmap::Pico::I2C0_SDA, GPIO_FUNC_I2C);
  gpio_set_function((uint)pinmap::Pico::I2C0_SCL, GPIO_FUNC_I2C);
  gpio_pull_up((uint)pinmap::Pico::I2C0_SDA);
  gpio_pull_up((uint)pinmap::Pico::I2C0_SCL);
}

// I2C Get Info Function
uint8_t chbsp_i2c_get_info(ch_group_t *grp_ptr, uint8_t dev_num, ch_i2c_info_t *info_ptr) {
  if (grp_ptr == NULL || info_ptr == NULL || dev_num >= grp_ptr->num_ports) {
    return 1;  // Error
  }
  
  ch_dev_t *dev_ptr = ch_get_dev_ptr(grp_ptr, dev_num);
  if (dev_ptr == NULL) {
    return 1;  // Error
  }
  
  // Fill in I2C information structure
  info_ptr->bus_num = 0;        // Using I2C0
  info_ptr->address = ch_get_i2c_address(dev_ptr);
  info_ptr->drv_flags = 0;      // No special flags
  
  return 0;  // Success
}

// SPI Functions (for ICU sensors)
void chbsp_spi_cs_on(ch_dev_t *dev_ptr) {
  // Not used in this implementation
}

void chbsp_spi_cs_off(ch_dev_t *dev_ptr) {
  // Not used in this implementation
}

int chbsp_spi_write(ch_dev_t *dev_ptr, const uint8_t *data,
                    uint16_t num_bytes) {
  // Not used in this implementation
  return 0;
}
}