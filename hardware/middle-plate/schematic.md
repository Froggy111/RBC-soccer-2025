Last updated: 11 October 2024

**PROGRESS**
- IR recievers have arrived. We should do some testing.
- Mirror shape has been numerically calculated. Preferably, we want to analytically solve the mirror equations to see what it ends up with.

**Controllers:**
NOTE: We might consider using a RPi 3B from Y14 instead (as they have a lot of them).
- RPi Zero 2 W
  - Used for computation, and interfacing camera and IMU. Also used for interfacing kicker and dribbler.
  - Pinouts (ALL UP TO CHANGE BASED ON ROUTING NEEDS):
    - MOSI: GPIO10
    - MISO: GPIO9
    - SCLK: GPIO11
    - bottom ESP32 CS: GPIO5
    - top ESP32 CS: GPIO6
    - ADC1 CS: GPIO25
    - ADC2 CS: GPIO16
    - ADC3 CS: GPIO26
    - TX to bottom and top ESP32: GPIO8 (TX)
    - RX to bottom ESP32: GPIO10 (RX)
    - bottom ESP32 EN: GPIO22
    - top ESP32 EN: GPIO19
    - dribbler LPWM: PWM0, GPIO12
    - dribbler RPWM: PWM1, GPIO13
    - dribbler EN: GPIO21
    - kicker TRIG: GPIO20
    - many unconnected pins

**Sensors:**
- IR recievers
  - Using TSSP4038 (up to change), as many other teams use it and it is made for IR sensing, not communication.
  - https://github.com/CoRoSoN-Kit/CoRoSoN-Kit/issues/25 
  https://yunit.techblog.jp/archives/86422131.html
  The IR ball emits 4 different intensities of IR following inverse square (1, 1/4, 1/16, 1/64) (meaning that the pulse-width drops should be linear to distance) which can then be smoothed with a simple RC low-pass filter into an analog signal, then through an ADC (MCP3008-I/P) to reduce the number of pins needed while not sacrificing much speed and accuracy.

**Power:**
This is the same as the bottom plate, other than that 3V3 and GND go up to the top plate.
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
IR sensors use 3V3, Pi uses 5V.