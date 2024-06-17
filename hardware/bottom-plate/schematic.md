Last updated: 4 June 2024

Bottom plate is below the motors, and provides the base structure for the bot. There is a ring of line sensors and 2 mouse sensors, interfaced with an ESP32. The ESP32 also outputs PWM to the motor drivers.

**SOFTWARE NOTES:**
- Refer to below pinouts.
- UART RX from Pi (middle plate) will recieve both data for bottom and top plate. Bottom plate recieves motor commands, and both plates recieve software switch commands, and various commands to reset sensors, etc.
- There are many simultaneous tasks for the ESP32. To increase performance, async with interrupts could be used.

**Controller:**
- ESP32-WROOM (30PIN)
  - Used for interfacing sensors
  - Pinouts (ALL UP TO CHANGE BASED ON ROUTING NEEDS):
    - sensor MOSI: GPIO13
    - sensor MISO: GPIO12
    - sensor SCLK: GPIO14
    - ADC1 CS: GPIO27
    - ADC2 CS: GPIO26
    - ADC3 CS: GPIO25
    - mouse 1 motion interrupt: GPIO39
    - mouse 1 CS: GPIO36
    - mouse 1 motion interrupt: GPIO15
    - mouse 1 CS: GPIO2
    - motor enable: GPIO34
    - motor 1 LPWM: GPIO35
    - motor 1 RPWM: GPIO32
    - motor 2 LPWM: GPIO33
    - motor 2 RPWM: GPIO22
    - motor 3 LPWM: GPIO1
    - motor 3 RPWM: GPIO3
    - motor 4 LPWM: GPIO21
    - motor 4 RPWM: GPIO4
    - TX/GPIO to Pi: GPIO17 (TX2)
    - RX to Pi: GPIO16 (RX2)
    - MOSI to Pi: GPIO23 (VSPI)
    - MISO to Pi: GPIO19 (VSPI)
    - SCLK to Pi: GPIO18 (VSPI)
    - CS by Pi: GPIO5 (VSPI)
    - EN passed up to Pi to turn off

**Sensors:**
- Line sensor ring
  - 24 Line sensors
    - 1 3.3V LED and 1 photoresistor each. LEDs are all chained together and connected to 1 resistor, to 3V3. Photoresistors can all share one power line as well.
    - Each group of 8 sensors are routed through an MCP3008-I/P ADC. There are 3 in total, and are interfaced through SPI. Refer to schematic for pins.
      - SPI is used to increase speeds and reduce possible issues with I2C.
      - Through hole is selected to make it easier to replace, as headers can be used.
- Mouse sensors
  - An existing, minimal mouse sensor breakout is used to increase modularity, and reduce errors as it works.
    - It is mounted directly above the bottom plate and screwed in. Connections are not determined yet but should be low footprint and easily removable.
    - Breakout is used also to reduce how much the mouse sensor + base plate sticks out, to allow the line sensor ring to be closer to the field.

**Motors:**
JST from PWM outputs to motor drivers, as there is no place to put motor drivers flush with bottom board (as mouse sensors take up the space).
Level shifting from 3.3V logic of ESP32 to 5V logic of motor drivers.

**Power:**
This is the same as the top plate.
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
This provides cleaned, protected power supply which will hopefully not fry any components.
Everything on the bottom board uses 3V3 other than level shifting to 5V for motor drivers.