Last updated: 4 June 2024

Top plate is above the mirror. Contains the IMU, Ultrasound ring and an ESP32 for interfacing.

**PROGRESS:**
- CH201 boards have been ordered, and will be here soon. Testing needs to be done and potentially overwriting the firmware of the CH201 sensors to make it have a smaller effective cone. Horn needs to be designed and printed.


**SOFTWARE NOTES:**
- Refer to below pinouts.
- CH201 has arduino library for interfacing. Edit as necessary to make it work with multiplexing.
- GPIO multiplexers have configurable INT triggers. That needs to be configured on startup, and when an interrupt is triggered, scan through the GPIO multiplexer interrupt registers until found what triggered the interrupt. This should be quite fast with SPI.
- UART RX from Pi (middle plate) will recieve both data for bottom and top plate. Bottom plate recieves motor commands, and both plates recieve software switch commands, and various commands to reset sensors, etc.
- There are many simultaneous tasks for the ESP32. To increase performance, async with interrupts could be used.

**Controllers:**
- ESP32-WROOM (30PIN)
  - Used for interfacing sensors
  - Pinouts (ALL UP TO CHANGE BASED ON ROUTING NEEDS):
    - sensor MOSI: GPIO13
    - sensor MISO: GPIO12
    - sensor SCLK: GPIO14
    - IMU CS: GPIO27
    - IMU RST: GPIO26
    - IMU INT: GPIO25
    - GPIO multiplexer CS: GPIO39
    - GPIO multiplexer reset: GPIO36
    - GPIO multiplexer 1 INT: GPIO4
    - GPIO multiplexer 2 INT: GPIO15
    - GPIO multiplexer 3 INT: GPIO35
    - GPIO multiplexer 4 INT: GPIO33
    - Level shifter 1 DIR: GPIO3
    - Level shifter 2 DIR: GPIO2
    - Level shifter 3 DIR: GPIO34
    - Level shifter 4 DIR: GPIO32
    - I2C SDA: GPIO21
    - I2C SCL: GPIO22
    - CH201 RESET: GPIO1
    - RX to Pi: GPIO16 (RX2)
    - TX to Pi: GPIO17 (TX2)
    - MOSI to Pi: VSPI, GPIO23
    - MISO to Pi: VSPI, GPIO19
    - SCLK to Pi: VSPI, GPIO18
    - CS by Pi: VSPI, GPIO5
    - EN passed down to Pi to turn off

**Sensors:**
- Ultrasounds
  - CH201 ultrasound sensor, with custom horn and enclosure to achieve narrow beam. Designed to have 24 of them in a ring such that a convex hull can be done to obtain position, angle and possible ball/bot positions. How many are populated depends on if price drops later. (currently at 12SGD per)
  - FFC connection from ultrasound breakouts to main board. I2C multiplexed with TCA9548A breakout, level shifted through autosensing 4 channel bidirectional level shifter (same with reset pins of TCA9548A (all chained together) and reset pins of CH201 (all chained together)). Interrupt and program pins are shifted to 3V3 through direction-controlled bidirectional level shifter (SN74LVCH8T245PWR (recommended by CH201 datasheet)), then multiplexed through MCP23S08, which uses SPI and has configurable interrupt conditions. Refer to schematics for more details.
  - Refer to ultrasounds docs for more details.
- IMU
  - BNO85 Adafruit breakout
  - Used for its high reliability
  - SPI connection as I2C is less stable

**Power:**
**Traces expected to carry large current should be thick!**
3V3 and GND is supplied from the middle plate.
That goes through decoupling capacitors, right next to the input port. 4 decoupling capacitors are placed to allow easy adjustment of decoupling capacity.
All have a probe at the unfiltered/noisy end and the filtered end to allow easier debugging.
The cleaned 3V3 goes into a 1.8V LDO then 4 decoupling capacitors, for the same reasoning. Again, probes for both the noisy and filtered end are placed.
This provides cleaned, protected power supply which will hopefully not fry any components.
IMU and ESP32 on top plate use 3V3, ultrasounds require 1.8V.