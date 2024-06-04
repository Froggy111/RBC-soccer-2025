Last updated: 3 June 2024

The top plate is a bit more complicated.

**Controllers:**
- ESP32-WROOM (30PIN)
  - Used for interfacing sensors
  - Pinouts (ALL UP TO CHANGE BASED ON ROUTING NEEDS):
    - sensor MOSI: GPIO13
    - sensor MISO: GPIO12
    - sensor SCLK: GPIO14
    - ADC1 CS: GPIO27
    - ADC2 CS: GPIO26
    - ADC3 CS: GPIO25
    - CH201 INT multiplexer CS: GPIO32
    - CH201 INT multiplexer INT: GPIO36
    - CH201 PROG multiplexer CS: GPIO33
    - CH201 PROG multiplexer INT: GPIO17
    - GPIO multiplexer reset: GPIO35
    - CH201 PROG level shifter DIR: GPIO39
    - I2C SCL: GPIO22
    - I2C SDA: GPIO21
    - CH201 RESET: GPIO1
    - I2C multiplexer RESET: GPIO3
    - general use pin to Pi: GPIO16
    - shifter 1 DIR: GPIO15
    - shifter 2 DIR: GPIO2 (TX0)
    - shifter 3 DIR: GPIO4 (RX0)
    - MOSI to Pi: VSPI, GPIO23
    - MISO to Pi: VSPI, GPIO19
    - SCLK to Pi: VSPI, GPIO18
    - CS by Pi: VSPI, GPIO5
    - EN passed up to Pi to turn off
- RPi Zero 2 W
  - Used for computation, and interfacing camera and IMU. Also used for interfacing kicker and dribbler.
  - Pinouts (ALL UP TO CHANGE BASED ON ROUTING NEEDS):
    - MOSI: GPIO10
    - MISO: GPIO9
    - SCLK: GPIO11
    - TX to bottom ESP32: GPIO8 (TX)
    - RX to bottom ESP32: GPIO10 (RX)
    - bottom ESP32 CS: GPIO5
    - top ESP32 CS: GPIO6
    - bottom ESP32 EN: GPIO22
    - top ESP32 EN: GPIO19
    - top ESP32 general use pin: GPIO26
    - IMU CS: GPIO18
    - IMU INT: GPIO16
    - IMU RESET: GPIO23
    - dribbler LPWM: PWM0, GPIO12
    - dribbler RPWM: PWM1, GPIO13
    - dribbler EN: GPIO21
    - kicker TRIG: GPIO20
    - unconnected:
      - GPIO24, GPIO25, GPIO8 (SPI0 CE0), GPIO7 (SPI0 CE1)

**Sensors:**
- IR recievers
  - Using TSSP4038 (up to change), as many other teams use it and it is made for IR sensing, not communication.
  - https://github.com/CoRoSoN-Kit/CoRoSoN-Kit/issues/25 
  https://yunit.techblog.jp/archives/86422131.html
  The IR ball emits 4 different intensities of IR following inverse square (1, 1/4, 1/16, 1/64) (meaning that the pulse-width drops should be linear to distance) which can then be smoothed with a simple RC low-pass filter into an analog signal, then through an ADC (MCP3008-I/P) to reduce the number of pins needed while not sacrificing much speed and accuracy.
- Ultrasounds
  - CH201 ultrasound sensor, with custom horn and enclosure to achieve narrow beam. Designed to have 24 of them in a ring such that a convex hull can be done to obtain position, angle and possible ball/bot positions. How many are populated depends on if price drops later. (currently at 12SGD per)
  - FFC connection from ultrasound breakouts to main board. I2C multiplexed with TCA9548A breakout, level shifted through autosensing 4 channel bidirectional level shifter (same with reset pins of TCA9548A (all chained together) and reset pins of CH201 (all chained together)). Interrupt and program pins are shifted to 3V3 through direction-controlled bidirectional level shifter (SN74LVCH8T245PWR (recommended by CH201 datasheet)), then multiplexed through MCP23S08, which uses SPI and has configurable interrupt conditions. Refer to schematics for more details.
  - Refer to ultrasounds docs for more details.

**Power:**
This is the same as the bottom plate, plus a 1V8 supply.
**Traces expected to carry large current should be thick!**
12V and GND from the LiPOs. The LiPOs should be connected together in parallel before connecting to the board.
That goes through decoupling capacitors, right next to the input port. 4 decoupling capacitors are placed to allow easy adjustment of decoupling capacity.
The filtered 12V is passed to a 5V buck and a 3V3 buck (Mini560). It is a high current breakout regulator, used mainly for the price.
It then goes through the same 4 decoupling capacitors, for the same reasoning.
All have a probe at the unfiltered/noisy end and the filtered end to allow easier debugging.
They then pass through a protection circuit. A crowbar made of a zener diode and triac is made to protect the circuit from sustained extremely high voltage (such as from the buck failing), by exceeding the zener voltage and activating the triac, shorting the circuit and blowing the unressetable fuse.
An unressetable fuse is used here to prevent possible oscillatory behavior with the circuit (fuse blows, current below triac holding current, traic shuts, resetting the crowbar and fuse resets, repeating)
The unressetable fuse blowing also means something really went wrong. (perhaps not enough decoupling capacity?)
There is then a resettable fuse after this crowbar, which protects the circuit from overcurrent. Small overcurrents will not require a replacement of the unressetable fuse.
The cleaned 3V3 goes into a 1.8V LDO then 4 decoupling capacitors, for the same reasoning. Again, probes for both the noisy and filtered end are placed.
This provides cleaned, protected power supply which will hopefully not fry any components.
IR recievers and ESP32 on top plate use 3V3, ultrasounds require 1.8V, Pi requires 5V (not technically, but disables some stuff that needs 5V, and not regulated on the Pi itself, if powered through 3V3.)