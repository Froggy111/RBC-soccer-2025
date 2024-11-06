#include <Arduino.h>
#include "main.hpp"

const int output_pin = 18;
const int freq = 40000;
const int period = 1000000 / freq;
const int waveform_freq = 1200;

const float samples_per_window = freq / waveform_freq;

Samples::Samples(int n_samples) {
  this->n_samples = n_samples;
  this->samples = (bool*) malloc(n_samples * sizeof(bool));
  memset((void*) this->samples, 0, n_samples * sizeof(bool));
  this->current_idx = 0;
  return;
}

void Samples::add(bool state) {
  this->samples[this->current_idx] = state;
  this->current_idx = (this->current_idx + 1) % n_samples;
  return;
}

float Samples::average(void) {
  int total = 0;
  for (int i = 0; i < n_samples; i++) {
    total += (int) this->samples[i];
  }
  float average = (float) total / (float) n_samples;
  return average;
}

Samples samples;

void setup() {
  Serial.println("\n\
 _____  ____   _____   _ __          _________   ___   ___ ___  _____ \n\
|  __ \\|  _ \\ / ____| | |\\ \\        / /__   __| |__ \\ / _ \\__ \\| ____|\n\
| |__) | |_) | |      | | \\ \\  /\\  / /   | |       ) | | | | ) | |__  \n\
|  _  /|  _ <| |      | |  \\ \\/  \\/ /    | |      / /| | | |/ /|___ \\ \n\
| | \\ \\| |_) | |____  | |___\\  /\\  /     | |     / /_| |_| / /_ ___) |\n\
|_|  \\_\\____/ \\_____| |______\\/  \\/      |_|    |____|\\___/____|____/ \n\
");
  Serial.begin(922190);
  pinMode(output_pin, INPUT);
  samples = Samples(samples_per_window);
  delay(2000);
}

void loop() {
  u_int64_t start_time = micros();
  bool state = digitalRead(output_pin);
  samples.add(state);
  Serial.print(">s:");
  Serial.println(state);
  // Serial.print(">a:");
  // Serial.println(samples.average());

  while (micros() - start_time < period) {
    continue;
  }
}
