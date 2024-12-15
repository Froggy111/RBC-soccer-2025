#include <Arduino.h>
char tx_buffer[64];
char rx_buffer[64];
void setup() {
  SerialUSB.begin(8000000); // Initialize USB CDC
}

void loop() {
  if (SerialUSB.available()) {
    int idx = 0;
    while (SerialUSB.available()) {
      rx_buffer[idx] = SerialUSB.read();
      idx += 1;
    }
    SerialUSB.write(rx_buffer, idx + 1);
    Serial.flush();
  }
}
