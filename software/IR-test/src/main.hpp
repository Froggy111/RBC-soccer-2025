class Samples {
public:
  Samples(void) {return;};
  Samples(int n_samples);
  void add(bool state);
  float average(void);
  inline void write_pin_low(uint8_t pin) {
    digitalWrite(pin, LOW);
  }
private:
  bool* samples;
  int n_samples;
  int current_idx;
};
