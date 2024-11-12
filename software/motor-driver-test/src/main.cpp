#include <SPI.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <queue>
#include <DRV8244.hpp>

DRV8244::DRV8244 motor_driver;

void setup()
{
  Serial.begin(115200);

  // Initialize Display

  // Set Pin Modes


  // Set SPI


  // Set device configurations
  
  // Set S_MODE = 0b11 for PWM mode

}

void loop()
{
}