#include <Arduino.h>
#include <SPI.h>
#include "typedefs.hpp"
#include "display.hpp"

namespace DRV8244 {

  #ifdef ARDUINO_ARCH_RP2040
  using GenericSPIClass = SPIClassRP2040;
  #elif defined(ARDUINO_ARCH_ESP32)
  using GenericSPIClass = SPIClass;
  #endif

  using namespace types;

  enum class DriverMode {
    uncertain = 0,
    ph_en = 1,
    pwm = 2,
    independent = 3,
  };

  enum class DriverState {
    uncertain = 0,
    sleep = 1,
    standby = 2,
    active = 3,
  };

  class DRV8244 {
  public:
    DRV8244(void);
    bool start(GenericSPIClass SPI, DriverMode driver_mode, DriverState driver_state, bool display_active);

    // * Other utility functions
    void write8(uint8_t reg, uint8_t value);
    void print_screen(std::string text, bool flush = true);

    // * For configuring driver
    bool set_pin_modes();
    bool set_spi();
    bool clear_fault();
    bool change_mode(DriverMode mode);

    // * For use in PWM mode
    float read_current();
    void set_speed(uint8_t speed); 

  private:
    GenericSPIClass _SPI;
    DriverMode _driver_mode;
    DriverState _driver_state;

    // "true" if we should print to display, false if we should print to serial
    bool _display_active;

    // Optional display controller
    Display _display_controller;
  };
}