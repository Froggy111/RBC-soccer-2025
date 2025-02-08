## Running the Bot
1. `mkdir build && cd build`
2. `cmake ..`
3. `make`
4. `./RBC-2025`

## To Monitor STDIO
- After attaching PICO, check `sudo dmesg | tail` for location where device attached 
	```
	[ 2298.260752] cdc_acm 1-11:1.0: ttyACM0: USB ACM device
	```
- Run `sudo screen /dev/ttyACM0 115200` to monitor the serial output
- `Control + A + D` to detach from screen