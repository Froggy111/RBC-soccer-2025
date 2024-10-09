#include <PMW3360.h>

/* 

# PIN CONNECTION
  * MI = MISO
  * MO = MOSI
  * SS = Slave Select / Chip Select
  * SC = SPI Clock
  * MT = Motion (active low interrupt line)
  * RS = Reset
  * GD = Ground
  * VI = Voltage in up to +5.5V 

Module   Arduino
  RS --- (NONE)
  GD --- GND
  MT --- (NONE)
  SS --- Pin_10   (use this pin to initialize a PMW3360 instance)
  SC --- SCK 
  MO --- MOSI
  MI --- MISO
  VI --- 5V

# PMW3360_DATA struct format and description
  - PMW3360_DATA.isMotion      : bool, True if a motion is detected. 
  - PMW3360_DATA.isOnSurface   : bool, True when a chip is on a surface 
  - PMW3360_DATA.dx, data.dy   : integer, displacement on x/y directions.
  - PMW3360_DATA.SQUAL         : byte, Surface Quality register, max 0x80
                               * Number of features on the surface = SQUAL * 8
  - PMW3360_DATA.rawDataSum    : byte, It reports the upper byte of an 18‐bit counter 
                               which sums all 1296 raw data in the current frame;
                               * Avg value = Raw_Data_Sum * 1024 / 1296
  - PMW3360_DATA.maxRawData    : byte, Max/Min raw data value in current frame, max=127
    PMW3360_DATA.minRawData
  - PMW3360_DATA.shutter       : unsigned int, shutter is adjusted to keep the average
                               raw data values within normal operating ranges.
 */

const int pmw3360_scs = 5;
const float inch_to_meters = 0.0254;

PMW3360 sensor;

int saneify_values(int raw_counts) {
  if (raw_counts > 65535 / 2) {return raw_counts - 65535;};
  return raw_counts;
}

void setup() {
  Serial.begin(9600);  
  while(!Serial);
  
  //sensor.begin(10, 1600); // to set CPI (Count per Inch), pass it as the second parameter
  if(sensor.begin(pmw3360_scs))  // 10 is the pin connected to SS of the module.
    Serial.println("Sensor initialization successed");
  else
    Serial.println("Sensor initialization failed");
  
  //sensor.setCPI(1600);    // or, you can set CPI later by calling setCPI();
}

int count_sum_x = 0;
int count_sum_y = 0;

void loop() {
  PMW3360_DATA data = sensor.readBurst();
  
  if(data.isOnSurface && data.isMotion)
  {
    uint CPI = sensor.getCPI();
    int count_x = saneify_values(data.dx);
    int count_y = - saneify_values(data.dy);
    count_sum_x += count_x;
    count_sum_y += count_y;
    Serial.print(count_sum_x);
    Serial.print("\t");
    Serial.println(count_sum_y);
  }
  
  delay(10);
}