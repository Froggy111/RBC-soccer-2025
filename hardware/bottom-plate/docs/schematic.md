The bottom plate is quite simple.

**Controller:**
- ESP32-WROOM (30PIN)

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
JST from PWM outputs to motor drivers, as there is no place to put motor drivers flush (as mouse sensors take up the space).
Level shifting from 3.3V logic of ESP32 to 5V logic of motor drivers.