#include <Adafruit_SSD1306.h>

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
  void initialize_display()
  {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
      Serial.println("SSD1306 allocation failed");
      stall_mcu();
    }
    Serial.println("SSD1306 allocation successful");

    // Initialize Display
        display.display();
    display.setTextSize(TEXT_SIZE);
    display.setTextColor(TEXT_COLOUR);
    display.setCursor(0, 0);
    display.println("Hello World!");
    display.display();
  }

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
    while (true)
    {
    };
  }
};