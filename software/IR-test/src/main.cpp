#include <Arduino.h>
#include "main.hpp"
#include "Teleplot.h"

const int output_pin = 18;
const float sample_frame_duration = 0.833; // number of miliseconds
const float samples_per_frame = 5;

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
Teleplot teleplot("127.0.0.1", 47269);

void setup() {
  Serial.println("\n\
 _____  ____   _____   _ __          _________   ___   ___ ___  _____ \n\
|  __ \\|  _ \\ / ____| | |\\ \\        / /__   __| |__ \\ / _ \\__ \\| ____|\n\
| |__) | |_) | |      | | \\ \\  /\\  / /   | |       ) | | | | ) | |__  \n\
|  _  /|  _ <| |      | |  \\ \\/  \\/ /    | |      / /| | | |/ /|___ \\ \n\
| | \\ \\| |_) | |____  | |___\\  /\\  /     | |     / /_| |_| / /_ ___) |\n\
|_|  \\_\\____/ \\_____| |______\\/  \\/      |_|    |____|\\___/____|____/ \n\
");
  Serial.begin(115200);
  pinMode(output_pin, INPUT);
  samples = Samples(samples_per_frame);
  delay(2000);
}

void loop() {
  bool state = digitalRead(output_pin);
  samples.add(state);
  Serial.println(state);
  Serial.println(samples.average());

  unsigned long currentTime = micros();
  teleplot.update("state", state);
  teleplot.update("time", currentTime);

  delay(sample_frame_duration / samples_per_frame);
}

