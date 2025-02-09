#include "hardware/gpio.h"
#include "hardware/spi.h"
#include <cstdint>
#include <pico/stdlib.h>

#define SPI_PORT0 spi0
#define SPI_PORT1 spi1

class MUX {
  int32_t MISO, MOSI, SCLK, CS;
  int32_t port_num;

public:
  MUX(int32_t miso, int32_t mosi, int32_t sclk, int32_t cs, int32_t port,
      int32_t baudrate) {
    MISO = miso, MOSI = mosi, SCLK = sclk, CS = cs;
    if (port == 0) {
      spi_init(SPI_PORT0, baudrate);
      port_num = 0;
    } else if (port == 1) {
      spi_init(SPI_PORT1, baudrate);
      port_num = 1;
    }

    gpio_set_function(MISO, GPIO_FUNC_SPI);
    gpio_set_function(SCLK, GPIO_FUNC_SPI);
    gpio_set_function(MOSI, GPIO_FUNC_SPI);

    gpio_init(CS);
    gpio_set_dir(CS, GPIO_OUT);
    gpio_put(CS, 1);
  }

  void write(int32_t bytes, uint8_t reg_address, uint8_t data[]) {
    uint8_t com_data[bytes + 1];
    com_data[0] = reg_address & 0x7F;
    for (int byte = 1; byte < bytes; byte++) {
      com_data[byte] = data[byte];
    }
    gpio_put(CS, 0);
    spi_write_blocking((port_num ? SPI_PORT1 : SPI_PORT0), com_data, bytes);
    gpio_put(CS, 1);
  }

  uint8_t read(uint8_t reg_address) {
    uint8_t buffer[1];
    uint8_t reg = reg_address | 0X80;
    gpio_put(CS, 0);
    spi_write_blocking((port_num ? SPI_PORT1 : SPI_PORT0), &reg, 1);
    spi_read_blocking((port_num ? SPI_PORT1 : SPI_PORT0), 0, buffer, 1);
    gpio_put(CS, 1);

    return buffer[0];
  }
};
