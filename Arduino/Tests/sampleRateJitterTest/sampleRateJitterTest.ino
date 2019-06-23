int analog_pin = A0;
const unsigned int sample_rate = 9000; //Hz
const unsigned int sample_n = 100;

unsigned long sample_period; //uS
unsigned long measured_sample_periods[sample_n]; //uS
unsigned long measured_sample_periods_sorted[sample_n]; //uS
unsigned long total_sample_time; //uS

unsigned int sample_bin;

void setup() {

  sample_period = 1000000 / sample_rate;

  Serial.begin(9600);
  while (!Serial) {} //wait for serial connection
  Serial.println("Serial connection inititated.");

  Serial.print("Desired sample rate: ");
  Serial.print(sample_rate);
  Serial.println(" Hz");

  Serial.print("Estimated sample period: ");
  Serial.print(sample_period);
  Serial.println(" uS");

  Serial.print("Desired number of samples: ");
  Serial.println(sample_n);

  Serial.print("Estimated total sample time: ");
  Serial.print(float(sample_n) / float(sample_rate));
  Serial.println(" S");
  Serial.println();
  
  Serial.println("Collecting sample...");

  unsigned long test_start_micros; //uS
  unsigned long test_end_micros; //uS
  unsigned long sample_start_micros; //uS
  unsigned long sample_end_micros; //uS
  test_start_micros = micros();
  for (int i = 0; i < sample_n; i++) {
    sample_start_micros = micros();
    sampling_function();
    while (micros() - sample_start_micros < sample_period) {}
    sample_end_micros = micros();
    measured_sample_periods[i] = sample_end_micros - sample_start_micros;
  }
  test_end_micros = micros();

  Serial.println("Sampling complete.");
  Serial.println("Analyzing results...");
  Serial.println();

  unsigned long total_sample_time = 0; //uS
  for (int i = 0; i < sample_n; i++) {
    total_sample_time += measured_sample_periods[i];
  }

  unsigned long mean_sample_period = total_sample_time / sample_n; //uS
  unsigned int mean_sample_rate = 1000000 / mean_sample_period; //Hz

  for (int i = 0; i < sample_n; i++) {
    measured_sample_periods_sorted[i] = measured_sample_periods[i];
  }
  bool sorted = false;
  while (sorted == false) {
    sorted = true;
    for (int i = 0; i < sample_n - 1; i++) {
      if (measured_sample_periods_sorted[i] > measured_sample_periods_sorted[i+1]) {
        sorted = false;
        unsigned long bubble = measured_sample_periods_sorted[i];
        measured_sample_periods_sorted[i] = measured_sample_periods_sorted[i+1];
        measured_sample_periods_sorted[i+1] = bubble;
      }
    }
  }

  unsigned long median_sample_period = measured_sample_periods_sorted[sample_n / 2]; //uS
  unsigned int median_sample_rate = 1000000 / median_sample_period; //Hz

  unsigned long min_sample_period = measured_sample_periods_sorted[0]; //uS
  unsigned long max_sample_period = measured_sample_periods_sorted[sample_n - 1]; //uS

  unsigned long total_test_time = test_end_micros - test_start_micros; //uS
  unsigned long test_overhead = total_test_time - total_sample_time; //uS

  Serial.println("Analytics");

  Serial.print("Mean sample period: ");
  Serial.print(mean_sample_period);
  Serial.println(" uS");
  
  Serial.print("Median sample period: ");
  Serial.print(median_sample_period);
  Serial.println(" uS");
  
  Serial.print("Shortest sample period: ");
  Serial.print(min_sample_period);
  Serial.println(" uS");
  
  Serial.print("Longest sample period: ");
  Serial.print(max_sample_period);
  Serial.println(" uS");

  Serial.print("Mean sample rate: ");
  Serial.print(mean_sample_rate);
  Serial.println(" Hz");
  
  Serial.print("Median sample rate: ");
  Serial.print(median_sample_rate);
  Serial.println(" Hz");

  Serial.print("Test overhead: ");
  Serial.print(test_overhead);
  Serial.println(" uS");
}

void loop() {
  
}

void sampling_function() {
  sample_bin = analogRead(analog_pin);
}
