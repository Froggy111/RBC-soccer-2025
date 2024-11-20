#include <SPI.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <queue>

// SPI Protocol
#define SPI_ADDRESS_MASK 0x3F00 // Mask for SPI register address bits
#define SPI_ADDRESS_POS 8       // Position for SPI register address bits
#define SPI_DATA_MASK 0x00FF    // Mask for SPI register data bits
#define SPI_DATA_POS 0          // Position for SPI register data bits
#define SPI_RW_BIT_MASK 0x4000  // Mask for SPI register read write indication bit

// PINS
// Arduino CLK = PIN 13
// Arduino SDI (MISO) = PIN 12
// Arduino SDO (MOSI) = PIN 11
#define CS_PIN 5
#define NSLEEP_PIN 15
#define NFAULT_PIN 4
#define IN2_PIN 13
#define IN1_PIN 16
#define DRVOFF_PIN 17
const uint8_t IPROPI_pin = 0;   // @jx put the IPROPI pin number here, also make sure its a pin that can read analog
const uint16_t A_IPROPI = 4750; // VQFN-HR package
const uint32_t IPROPI_resistance = 680;
const uint32_t ipropi_moving_average_size = 16384; // change this if uw, if its too big it wont fit

// Constants for display parameters
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int SCREEN_SDA = 0;
const int SCREEN_SDL = 1;
const int TEXT_SIZE = 1;
const int TEXT_COLOUR = WHITE;

class Display
{
public:
  Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1, 400000UL, 100000UL);
  void print_screen(std::string text, bool flush = true)
  {
    if (flush)
    {
      display.setCursor(0, 0);
      display.clearDisplay();
    }
    Serial.println(text.c_str());
    display.println(text.c_str());
    display.display();
    return;
  }

  void print_screen_hex(uint8_t data, bool flush = true)
  {
    if (flush)
    {
      display.setCursor(0, 0);
      display.clearDisplay();
    }
    Serial.println(data, HEX);
    display.println(data, HEX);
    display.display();
    return;
  }

  void print_screen_hex(uint16_t data, bool flush = true)
  {
    if (flush)
    {
      display.setCursor(0, 0);
      display.clearDisplay();
    }
    Serial.println(data, HEX);
    display.println(data, HEX);
    display.display();
    return;
  }

  void stall_mcu(void)
  {
    while (true) {};
  }

  void initialize_display()
  {
    display.display();
    display.setTextSize(TEXT_SIZE);
    display.setTextColor(TEXT_COLOUR);
    display.setCursor(0, 0);
    display.println("Hello World!");
    display.display();
  }
};

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

class MotorDriver
{
public:
  MovingAverage<float, ipropi_moving_average_size> motor_current;
  Display display_controller;
  Adafruit_SSD1306 display = display_controller.display;
  /**
   * This function is used to read the current from the IPROPI pin.
   */
  float read_current()
  {
    float voltage = ((float)analogReadMilliVolts(IPROPI_pin)) / 1000;
    float IPROPI_current = voltage / IPROPI_resistance;
    float current = IPROPI_current * A_IPROPI; // only one high side is recorded in PWM mode, so its correct
    return current;
  }

  /**
   * This SPI function is used to write the set device configurations and operating
   * parameters of the device.
   * Register format |R/W|A5|A4|A3|A2|A1|A0|*|D7|D6|D5|D4|D3|D2|D1|D0|
   * Ax is address bit, Dx is data bits and R/W is read write bit.
   * For write R/W bit should 0.
   */
  void write8(uint8_t reg, uint8_t value)
  {
    volatile uint16_t reg_value = 0;                            // Variable for the combined register and data info
    reg_value |= ((reg << SPI_ADDRESS_POS) & SPI_ADDRESS_MASK); // Adding register address value
    reg_value |= ((value << SPI_DATA_POS) & SPI_DATA_MASK);     // Adding data value

    digitalWrite(CS_PIN, LOW);

    SPI.transfer((uint8_t)((reg_value >> 8) & 0xFF));
    uint16_t recieved = SPI.transfer((uint8_t)(reg_value & 0xFF));
    display_controller.print_screen_hex((uint8_t)(recieved), false);
    display_controller.print_screen_hex((uint8_t)(recieved >> 8), false);

    digitalWrite(CS_PIN, HIGH);
  }
};

MotorDriver motor_driver;

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
  float current, average;

  // Accelerate by increasing the PWM duty cycle
  for (int i = 0; i < 255; i++)
  {
    analogWrite(IN2_PIN, i);
    delay(2);
    current = motor_driver.read_current();
    average = motor_driver.motor_current.add(current);
  }

  // Decelerate by decreasing the PWM duty cycle
  for (int i = 255; i > 0; i--)
  {
    analogWrite(IN2_PIN, i);
    delay(2);
    current = motor_driver.read_current();
    average = motor_driver.motor_current.add(current);
  }
}