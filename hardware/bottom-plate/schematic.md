# Bottom plate

Last updated: 24 December 2024

The bottom plate is the main PCB of the bot. It contains everything that drives the robot (motors, dribbler, kicker), with plug-in motor drivers, and sensors (line sensors, mouse sensors, interface for a lightgate and loadcell sensor).

## Progress

- Mouse sensor works. Drift is acceptable. Concern is the distance scaling with how far it is from the ground, that needs to be calibrated and maybe compensated for by measuring surface quality.
- Motor drivers work. They might get pretty hot, so leave space to put heatsinks on them.
- Redesign in progress. A 4 layer PCB is chosen to increase density, so more of the PCB can be cut out to save mass.

## Software notes

- Refer to schematic for some details.
- Generally, the hardware has been designed to minimise overhead where possible.
- The main controller (rpi zero 2w) connects to the bottom plate via USB and/or UART (UART for very low-latency tasks, and as a backup in case USB-CDC latencies are somehow very high (shouldn't be the case).)

## Controller

RPi pico (or some other drop-in replacement), preferably USB-C version because microusb connectors suck.  
Pushbuttons are added for EN and RUN pins, to facilitate easier testing. They are also exposed along with the UART pins in a 4 pin JST-XH connector to the middle plate.  
A pico is chosen for its reliable performance and lack of gimmicks (unlike the ESP32), which results in more usable pins (still ends up being just enough!).

## Sensors

Line sensor ring with 48 line sensors.  
Each line sensor contains:
- LED (LCSC C219231, white (6000-7000K), 2.8-3.4V, 1400mcd @20mA) (should be very bright)
- LED current limiting resistor (220Ohm to limit to ~15mA)
- Photoresistor (ALS-PT19).
- 4.7k resistor for the potential divider of ALS-PT19

Each group of 16 sensors is routed through an analog multiplexer (CD74HC4067M). There are 3 in total (as there are 3 ADC pins exposed on a pico). Their decoder pins are connected to digital multiplexers. Refer to schematic for pins.  

Mouse sensors are mounted directly below the bottom plate and screwed in. Uses a 1mm pitch FFC connector for relatively easier soldering, flexibility and compactness. Refer to schematic for pins.  
Ground clearance is not much, and the baseplate would be thinner than recommended. This is probably fine though (and can be fixed by using a 0.8mm pcb for the mouse sensor instead).  
Note: either an opposite side FFC cable, or an opposite side FFC connector has to be used.  

## Motor drivers

DRV8244 breakout boards, uses headers for connection to board, plug-in motor drivers for easy swapping. Configured through SPI. Has onboard XT-30 output to motor.  
DRV1-4 are for the drivebase. DRV5 is for the dribbler motor (a BE-3561 or some other brushed DC motor would likely be used for the dribbler).

## Kicker

Seperate kicker PCB, gets power and signal from bottom plate through headers. Capacitors might stick up too high, so cutouts need to be made on the middle plate.

## Power

There is no space for circuit protection (the max current is too high for small automotive fuses).  
Circuit protection is achieved by having connectors that are not pluggable in reverse (XT series, JST series).  
Sufficient filter capacitances are chosen to smooth out voltage and critically damp circuit inductance when switched on.  
Mini560 buck converter is used for 5V supply. A 1A 3.3V LDO (TLV1117) is used for 3.3V supply. Pico is fed 5V through VSYS (recommended way of powering pico). There is a master toggle switch at the input of the Mini560 buck, to cut all power to the controls of the bottom plate.
