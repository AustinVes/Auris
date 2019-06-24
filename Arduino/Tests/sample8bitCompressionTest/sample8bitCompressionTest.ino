const int analog_pin = A0;

const unsigned int sample_n = 200;
unsigned int raw_samples[sample_n]; //array of 16-bit values to hold 10-bit reading from ADC
byte compressed_samples[sample_n]; //array of 8-bit values to hold 10-bit reading from ADC
unsigned int compressed_sample_deviance[sample_n]; //% error between raw sample and compressed sample

void setup() {

  Serial.begin(9600);
  while (!Serial) {} //wait for serial connection
  Serial.println("Serial connection initiated.");

  Serial.print("Collecting ");
  Serial.print(sample_n);
  Serial.println(" samples...");

  for (int i = 0; i < sample_n; i++) {
    raw_samples[i] = analogRead(analog_pin);
  }

  Serial.println("Sampling complete.");
  Serial.println("Compressing samples...");

  for (int i = 0; i < sample_n; i++) {
    compressed_samples[i] = raw_samples[i]>>2;
  }

  Serial.println("Compression complete.");
  Serial.println("Analyzing loss...");
  Serial.println();

  for (int i = 0; i < sample_n; i++) {
    unsigned int decompressed_sample = int(compressed_samples[i])<<2;
    unsigned int difference = abs(raw_samples[i] - decompressed_sample);
    compressed_sample_deviance[i] = (difference * 100) / abs(raw_samples[i] - 1024/2);
    if (compressed_sample_deviance[i] == 65535) {
      compressed_sample_deviance[i] = 0; //fixing divide by zero error
    }
  }

  unsigned int summed_deviance = 0;
  for (int i = 0; i < sample_n; i++) {
    summed_deviance += compressed_sample_deviance[i];
  }

  unsigned int mean_sample_loss = summed_deviance / sample_n;

  bool sorted = false;
  while (sorted == false) {
    sorted = true;
    for (int i = 0; i < sample_n - 1; i++) {
      if (compressed_sample_deviance[i] > compressed_sample_deviance[i+1]) {
        sorted = false;
        unsigned int bubble = compressed_sample_deviance[i];
        compressed_sample_deviance[i] = compressed_sample_deviance[i+1];
        compressed_sample_deviance[i+1] = bubble;
      }
    }
  }

  unsigned int median_sample_loss = compressed_sample_deviance[sample_n / 2];
  unsigned int min_sample_loss = compressed_sample_deviance[0];
  unsigned int max_sample_loss = compressed_sample_deviance[sample_n -1];

  Serial.println("Analytics");

  Serial.print("Mean loss: ");
  Serial.print(mean_sample_loss);
  Serial.println("%");

  Serial.print("Median loss: ");
  Serial.print(median_sample_loss);
  Serial.println("%");

  Serial.print("Lowest loss: ");
  Serial.print(min_sample_loss);
  Serial.println("%");

  Serial.print("Greatest loss: ");
  Serial.print(max_sample_loss);
  Serial.println("%");

}

void loop() {

}
