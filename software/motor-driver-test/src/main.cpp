#include <SPI.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <queue>

// PINS
// Arduino CLK = PIN 13
// Arduino SDI (MISO) = PIN 12
// Arduino SDO (MOSI) = PIN 11
#define CS_PIN 5
#define NSLEEP_PIN 15
#define NFAULT_PIN 4
#define IN1_PIN 16
#define DRVOFF_PIN 17


// Brushed DC Motor Connected between OUT1 and OUT2 on DRV8244S-Q1EVM
// All jumpers on J4 removed except IPROPI
template <typename T, size_t SIZE>
class MovingAverage
{
private:
  T buffer[SIZE]; // Ring buffer to store values
  size_t index;   // Current position in buffer
  size_t count;   // Number of values currently in buffer
  T sum;          // Running sum of values

public:
  MovingAverage() : index(0), count(0), sum(0)
  {
    memset(buffer, 0, sizeof(buffer));
  }

  /**
   * Add a new value and return the current average
   */
  T add(T value)
  {
    // Subtract the oldest value from sum if buffer is full
    if (count == SIZE)
    {
      sum -= buffer[index];
    }
    else
    {
      count++;
    }

    // Add new value
    buffer[index] = value;
    sum += value;

    // Move index
    index = (index + 1) % SIZE;

    // Return current average
    return get();
  }

  /**
   * Get the current average
   */
  T get() const
  {
    if (count == 0)
      return 0;
    return sum / count;
  }

  /**
   * Reset the buffer
   */
  void reset()
  {
    memset(buffer, 0, sizeof(buffer));
    index = 0;
    count = 0;
    sum = 0;
  }

  /**
   * Get the number of values in the buffer
   */
  size_t size() const
  {
    return count;
  }

  /**
   * Check if the buffer is full
   */
  bool isFull() const
  {
    return count == SIZE;
  }
};


MotorDriver motor_driver;
MovingAverage<float, ipropi_moving_average_size> motor_average;

void setup()
{
  Serial.begin(115200);

  // Check if display is connected
  if (!motor_driver.display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("SSD1306 allocation failed");
    motor_driver.display_controller.stall_mcu();
  }
  Serial.println("SSD1306 allocation successful");

  // Initialize Display
  motor_driver.display_controller.initialize_display();

  // Set pin modes
  motor_driver.display_controller.print_screen("Setting pin modes", false);
  pinMode(NSLEEP_PIN, OUTPUT);
  digitalWrite(NSLEEP_PIN, 1);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, 1);
  pinMode(IN2_PIN, OUTPUT);
  digitalWrite(IN2_PIN, 0);
  pinMode(IN1_PIN, OUTPUT);
  digitalWrite(IN1_PIN, 0);
  pinMode(NFAULT_PIN, OUTPUT);
  digitalWrite(NFAULT_PIN, 0);
  pinMode(DRVOFF_PIN, OUTPUT);
  digitalWrite(DRVOFF_PIN, 0);
  motor_driver.display_controller.print_screen("Set pin modes", false);

  delay(5);

  // Set SPI
  motor_driver.display_controller.print_screen("Setting SPI", false);
  SPI.begin(); // initialize the SPI library
  SPI.setDataMode(SPI_MODE1);

  // Set device configurations
  motor_driver.display_controller.print_screen("Sending commands");
  motor_driver.write8(0x08, 0b10000000); // Send CLR_FLT command
  delay(5);

  // Set S_MODE = 0b11 for PWM mode
  motor_driver.display_controller.print_screen("Enter PWM mode", false);
  motor_driver.write8(0x0C, 0b01000011);
  digitalWrite(DRVOFF_PIN, 0);
}

void loop()
{
}