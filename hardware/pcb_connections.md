RPi pins: https://pinout.xyz

3v3 power plane	

2 5V-in pins and ground plane should be connected to LiPo (DONE)
all gnd pins to gnd plane? (DONE)
GPIO 2 (SDA) and GPIO 3 (SCL) should go to 2 or 3 multiplexers (depending on if one is 16x or 8x) (ADC multiplexes anyways so yeah)(DONE)
1 or 2 general GPIO pins should go to multiplexers to set I2C address (DONE)
GPIO 12, 13 for PWM to 1 motor (DONE)
use software PWM to other 3 motors (DONE)
and that is kinda it? there are a lot of spare pins (actually only 4 if not using the "special pins", or 12 if not using the SPI pins (idk what they do though))


2 PCBs - bottom one with RPi5 and TEMTs(AO) and LEDs
top one with RPi Zero W with USes(AO) and TSOP382(38kHz DO)

can send I2C from bottom plate to top plate for USes maybe? or just use the zero w with another ADC (will need another ADC anyways) (probably just use the I2C on the zero w)

make sure cam FPCs (the cable, flexible printed circuit or smth) are long enough!

microUSB between zero w and pi5 - transmits data of 1 cam, _ IRs, 4(?) USes (unconfirmed)

Pi5 might need to be on top if height constraints (should not be an issue)

should probably be it? idk... bottom board is basically done (?), just make top board

and then we solder everything :D