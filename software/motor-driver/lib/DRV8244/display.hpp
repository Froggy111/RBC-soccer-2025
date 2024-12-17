#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <string>

class Display
{
public:
  Display();
  void print_screen(std::string text, bool flush = true);
  void print_screen_hex(uint8_t data, bool flush = true);
  void print_screen_hex(uint16_t data, bool flush = true);
  void stall_mcu(void);

private:
  Adafruit_SSD1306 display;
};

#endif // DISPLAY_HPP