/*
 *
 * Copyright (c) 2024 by InvenSense, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "esp32-hal-gpio.h"
#include <CH201.h>
namespace pins {
uint8_t INT = 16;
uint8_t RST = 17;
uint8_t PROG = 18;
}

// CH201_dev CHx01_dev(Wire, 0x29, 16, 5, 18);
CH201 CHx01(Wire, pins::INT, 5, pins::RST, pins::PROG, false);

void setup() {
  int ret;
  Serial.begin(115200);
  while (!Serial);
  Serial.println("CH201 Raw Data print");

  // read pins
  pinMode(pins::INT, INPUT);
  pinMode(pins::RST, INPUT);
  pinMode(pins::PROG, INPUT);
  while (true) {
    Serial.printf("%hhu, %hhu, %hhu\n", digitalRead(pins::INT), digitalRead(pins::RST), digitalRead(pins::PROG));
  }

  // Initializing the CHx01 device
  ret = CHx01.begin();
  Serial.println("CH201 initialisation call done");
  if (ret != 0) {
    Serial.print("CH201 initialization failed: ");
    Serial.println(ret);
    while (1)
      ;
  } else {
    Serial.println("Printing informations");
    CHx01.print_informations();
    Serial.println("Printed informations");
  }
  // Start CHx01 in free run mode (max range 2.4m, interval 200ms)
  Serial.println("Starting CH201 in free run");
  ret = CHx01.free_run(2400,200);
  if (ret != 0) {
    Serial.print("CH201 free run failed: ");
    Serial.println(ret);
    while (1)
      ;
  } else {
    Serial.println("Printing configuration");
    CHx01.print_configuration();
    Serial.println("Printed configuration");
  }
}

void loop() {
  ch_iq_sample_t raw_data[CH201_MAX_NUM_SAMPLES];
  uint16_t nb_samples;

  /* Wait for new measure done */
  if (CHx01.data_ready()) {
    /* Get raw data from the sensor */
    CHx01.get_iq_data(raw_data, nb_samples);
    Serial.println("CH201 Raw Data (clear)");
    for (int count = 0; count < nb_samples; count++) {
      /* output one I/Q pair per line */
      Serial.print("q=");
      Serial.print(raw_data[count].q);
      Serial.print(",i=");
      Serial.println(raw_data[count].i);
    }
  }
}
