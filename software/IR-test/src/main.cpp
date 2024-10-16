#include <Arduino.h>

const int output_pin = 22;

const float sample_duration = 0.1; // in seconds
const int samples_per_second = 40;
const int n_samples = sample_duration * samples_per_second;
const float delay_time = 1000 / samples_per_second;

class Samples {
public:
  Samples(void) {return;};
  Samples(int n_samples);
  void add(bool state);
  float average(void);
private:
  bool* samples;
  int n_samples;
  int current_idx;
  int current_n_samples;
};

Samples samples;

void setup() {
  Serial.begin(115200);
  pinMode(output_pin, INPUT);
  samples = Samples(n_samples);
  delay(1000);
  Serial.println("Setup completed");
}

void loop() {
  bool state = digitalRead(output_pin);
  // samples.add(state);
  // Serial.println(samples.average());
  Serial.println("Looping");
  Serial.println(samples.average());
  delay(delay_time);
}

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
  this->current_n_samples += 1;
  if (this->current_n_samples > n_samples) {
    this->current_n_samples = n_samples;
  }
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