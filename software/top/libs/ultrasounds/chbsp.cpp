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
void chbsp_print_str(const char *str) { 
  comms::USB_CDC.printf("FUNC: chbsp_print_str\r\n");
  comms::USB_CDC.printf("%s", str); 
}

void chbsp_debug_toggle(uint8_t dbg_pin_num) {
  comms::USB_CDC.printf("FUNC: chbsp_debug_toggle(%u)\r\n", dbg_pin_num);
  // NOT USED
}

// Init Pins
void chbsp_init() {
  comms::USB_CDC.printf("FUNC: chbsp_init\r\n");
  // Initialize DMUX
  dmux1->init(1, spi0);
  dmux2->init(2, spi0);

  // Init all dmux pins to output, and 1
  for (int i = 0; i < 32; i++) {
    // the sleep is to increase dmux reliability, else it sometimes fails
    if (i < 16) {
      dmux1->init_gpio(i % 8, i < 8, 1);
      sleep_ms(25);
      dmux1->write_gpio(i % 8, i < 8, 1);
      sleep_ms(25);
    } else {
      dmux2->init_gpio(i % 8, i < 24, 1);
      sleep_ms(25);
      dmux2->write_gpio(i % 8, i < 24, 1);
      sleep_ms(25);
    }
  }
}

// Reset Functions (CH101/CH201)
void chbsp_reset_assert(void) { 
  comms::USB_CDC.printf("FUNC: chbsp_reset_assert\r\n");
  gpio_put((uint)pinmap::Pico::US_NRST, 0); 
}

void chbsp_reset_release(void) { 
  comms::USB_CDC.printf("FUNC: chbsp_reset_release\r\n");
  gpio_put((uint)pinmap::Pico::US_NRST, 1); 
}

// PROG Pin Control (CH101/CH201)
void chbsp_program_enable(ch_dev_t *dev_ptr) {
  uint8_t id = ch_get_dev_num(dev_ptr);
  comms::USB_CDC.printf("FUNC: chbsp_program_enable(dev_id=%u)\r\n", id);

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
  comms::USB_CDC.printf("FUNC: chbsp_program_disable(dev_id=%u)\r\n", id);

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
  comms::USB_CDC.printf("FUNC: chbsp_set_int1_dir_out(dev_id=%u)\r\n", id);

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
  comms::USB_CDC.printf("FUNC: chbsp_group_set_int1_dir_out(num_ports=%u)\r\n", grp_ptr->num_ports);
  for (uint8_t i = 0; i < grp_ptr->num_ports; i++) {
    ch_dev_t *dev_ptr = ch_get_dev_ptr(grp_ptr, i);
    if (dev_ptr != NULL) {
      chbsp_set_int1_dir_out(dev_ptr);
    }
  }
}

void chbsp_int1_clear(ch_dev_t *dev_ptr) {
  uint8_t id = ch_get_dev_num(dev_ptr);
  comms::USB_CDC.printf("FUNC: chbsp_int1_clear(dev_id=%u)\r\n", id);

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
  comms::USB_CDC.printf("FUNC: chbsp_group_int1_clear(num_ports=%u)\r\n", grp_ptr->num_ports);
  for (uint8_t i = 0; i < grp_ptr->num_ports; i++) {
    ch_dev_t *dev_ptr = ch_get_dev_ptr(grp_ptr, i);
    if (dev_ptr != NULL) {
      chbsp_int1_clear(dev_ptr);
    }
  }
}

void chbsp_int1_set(ch_dev_t *dev_ptr) {
  uint8_t id = ch_get_dev_num(dev_ptr);
  comms::USB_CDC.printf("FUNC: chbsp_int1_set(dev_id=%u)\r\n", id);

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
  comms::USB_CDC.printf("FUNC: chbsp_group_int1_set(num_ports=%u)\r\n", grp_ptr->num_ports);
  for (uint8_t i = 0; i < grp_ptr->num_ports; i++) {
    ch_dev_t *dev_ptr = ch_get_dev_ptr(grp_ptr, i);
    if (dev_ptr != NULL) {
      chbsp_int1_set(dev_ptr);
    }
  }
}

// I2C Initialization
int chbsp_i2c_init(void) {
  comms::USB_CDC.printf("FUNC: chbsp_i2c_init\r\n");
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
void chbsp_delay_us(uint32_t us) { 
  comms::USB_CDC.printf("FUNC: chbsp_delay_us(%lu)\r\n", us);
  sleep_us(us); 
}

void chbsp_delay_ms(uint32_t ms) { 
  comms::USB_CDC.printf("FUNC: chbsp_delay_ms(%lu)\r\n", ms);
  sleep_ms(ms); 
}

// Timestamp
uint32_t chbsp_timestamp_ms(void) {
  comms::USB_CDC.printf("FUNC: chbsp_timestamp_ms\r\n");
  return to_ms_since_boot(get_absolute_time());
}

// * I2C Functions
// I2C Write Function
int chbsp_i2c_write(ch_dev_t *dev_ptr, const uint8_t *data,
                    uint16_t num_bytes) {
  uint8_t addr = ch_get_i2c_address(dev_ptr);
  comms::USB_CDC.printf("FUNC: chbsp_i2c_write(addr=0x%02X, len=%d)\r\n", addr, num_bytes);
  comms::USB_CDC.printf("I2C Write: addr=0x%02X len=%d\r\n", addr, num_bytes);

  int result = i2c_write_blocking(i2c0, addr, data, num_bytes, false);
  if (result != num_bytes) {
    comms::USB_CDC.printf("I2C Write Failed: expected=%d actual=%d\r\n",
                          num_bytes, result);
    return 1;
  }
  return 0;
}

// I2C Memory Write Function
int chbsp_i2c_mem_write(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data,
                        uint16_t num_bytes) {
  uint8_t addr = ch_get_i2c_address(dev_ptr);
  comms::USB_CDC.printf("FUNC: chbsp_i2c_mem_write(addr=0x%02X, mem_addr=0x%04X, len=%d)\r\n", 
                        addr, mem_addr, num_bytes);
  uint8_t buf[num_bytes + 2];
  buf[0] = (mem_addr >> 8) & 0xFF;
  buf[1] = mem_addr & 0xFF;
  memcpy(buf + 2, data, num_bytes);
  return chbsp_i2c_write(dev_ptr, buf, num_bytes + 2);
}

// I2C Read Function
int chbsp_i2c_read(ch_dev_t *dev_ptr, uint8_t *data, uint16_t num_bytes) {
  uint8_t addr = ch_get_i2c_address(dev_ptr);
  comms::USB_CDC.printf("FUNC: chbsp_i2c_read(addr=0x%02X, len=%d)\r\n", addr, num_bytes);
  comms::USB_CDC.printf("I2C Read: addr=0x%02X len=%d\r\n", addr, num_bytes);

  int result = i2c_read_blocking(i2c0, addr, data, num_bytes, false);
  if (result != num_bytes) {
    comms::USB_CDC.printf("I2C Read Failed: expected=%d actual=%d\r\n",
                          num_bytes, result);
    return 1;
  }
  return 0;
}

// I2C Memory Read Function
int chbsp_i2c_mem_read(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data,
                       uint16_t num_bytes) {
  uint8_t addr = ch_get_i2c_address(dev_ptr);
  comms::USB_CDC.printf("FUNC: chbsp_i2c_mem_read(addr=0x%02X, mem_addr=0x%04X, len=%d)\r\n", 
                        addr, mem_addr, num_bytes);
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
  uint8_t addr = ch_get_i2c_address(dev_ptr);
  comms::USB_CDC.printf("FUNC: chbsp_i2c_reset(addr=0x%02X)\r\n", addr);
  // Reset GPIO functions for I2C pins
  gpio_set_function((uint)pinmap::Pico::I2C0_SDA, GPIO_FUNC_I2C);
  gpio_set_function((uint)pinmap::Pico::I2C0_SCL, GPIO_FUNC_I2C);
  gpio_pull_up((uint)pinmap::Pico::I2C0_SDA);
  gpio_pull_up((uint)pinmap::Pico::I2C0_SCL);
}

// I2C Get Info Function
uint8_t chbsp_i2c_get_info(ch_group_t *grp_ptr, uint8_t dev_num, ch_i2c_info_t *info_ptr) {
  comms::USB_CDC.printf("FUNC: chbsp_i2c_get_info(dev_num=%u)\r\n", dev_num);
  if (grp_ptr == NULL || info_ptr == NULL || dev_num >= grp_ptr->num_ports) {
    return 1; // Error
  }

  ch_dev_t *dev_ptr = ch_get_dev_ptr(grp_ptr, dev_num);
  
  // Hardcode I2C info during initialization if dev_ptr is NULL
  if (dev_ptr == NULL) {
    // Use default values during initialization
    info_ptr->bus_num = 0;   // I2C0
    info_ptr->address = 0x45; // Default CH201 address
    info_ptr->drv_flags = 0;
    return 0; // Success with defaults
  }

  // Fill in I2C information structure with actual device info
  info_ptr->bus_num = 0;
  info_ptr->address = ch_get_i2c_address(dev_ptr);
  info_ptr->drv_flags = 0;

  return 0; // Success
}

// SPI Functions (for ICU sensors)
void chbsp_spi_cs_on(ch_dev_t *dev_ptr) {
  uint8_t id = ch_get_dev_num(dev_ptr);
  comms::USB_CDC.printf("FUNC: chbsp_spi_cs_on(dev_id=%u)\r\n", id);
  // Not used in this implementation
}

void chbsp_spi_cs_off(ch_dev_t *dev_ptr) {
  uint8_t id = ch_get_dev_num(dev_ptr);
  comms::USB_CDC.printf("FUNC: chbsp_spi_cs_off(dev_id=%u)\r\n", id);
  // Not used in this implementation
}

int chbsp_spi_write(ch_dev_t *dev_ptr, const uint8_t *data,
                    uint16_t num_bytes) {
  uint8_t id = ch_get_dev_num(dev_ptr);
  comms::USB_CDC.printf("FUNC: chbsp_spi_write(dev_id=%u, len=%d)\r\n", id, num_bytes);
  // Not used in this implementation
  return 0;
}
}