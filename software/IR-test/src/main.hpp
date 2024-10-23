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
};
