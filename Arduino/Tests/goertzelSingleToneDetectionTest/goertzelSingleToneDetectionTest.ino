// based on the optimized Goertzel algorithm described here: https://www.embedded.com/design/configurable-systems/4024443/The-Goertzel-Algorithm

const int audio_pin = A0;

const unsigned int target_frequency = 400; // Hz
const unsigned int sample_rate = 1200; // Hz
//const unsigned int block_size = 300; // <-- bin width = 4Hz, sample time = 0.25s
const int block_size = 120; // <-- bin width = 10Hz, sample time = 0.1s

// gtzl is short for Goertzel

const int gtzl_k = (int) 0.5 + (block_size * target_frequency / sample_rate);
const double gtzl_w = (TWO_PI / block_size) * gtzl_k;
const double gtzl_cosine = cos(gtzl_w);
const double gtzl_coeff = 2 * gtzl_cosine;

double Q0;
double Q1;
double Q2;

int sample;
unsigned long sample_period = 1000000 / sample_rate; // uS
unsigned long sample_start; // uS
unsigned long block_start; // uS
unsigned int measured_sample_rate; // Hz
double magnitude;

void setup() {

  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("Serial connection initiated.\n");
  Serial.println(sample_period);

  Serial.print("Target frequency: ");
  Serial.print(target_frequency);
  Serial.println(" Hz");

  Serial.print("Sampling rate: ");
  Serial.print(sample_rate);
  Serial.println(" Hz");

  Serial.print("Block size: ");
  Serial.println(block_size);

  Serial.print("Bin width: ");
  Serial.print(float(sample_rate) / float(block_size));
  Serial.println(" Hz");

  Serial.print("Sampling time: ");
  Serial.print(float(block_size) / float(sample_rate));
  Serial.println("s");

  Serial.print("k = ");
  Serial.println(gtzl_k);

  Serial.print("w = ");
  Serial.println(gtzl_w);

  Serial.print("cosine = ");
  Serial.println(gtzl_cosine);

  Serial.print("coeff = ");
  Serial.println(gtzl_coeff);

  Serial.println();

  Serial.println("SAMPLING IN");
  for (int i = 5; i > 0; i--) {
    Serial.println(i);
    delay(1000);
  }
  Serial.println();
  
}

void loop() {

  sample = 0;
  Q0 = 0;
  Q1 = 0;
  Q2 = 0;

  block_start = micros();
  for (int i = 0; i < block_size; i++){
    sample_start = micros();

    sample = analogRead(audio_pin) - 512; // center 10-bit ADC reading around 0
    Q0 = gtzl_coeff * Q1 - Q2 + sample;
    Q2 = Q1;
    Q1 = Q0;
    
    while (micros() - sample_start < sample_period) {}
  }
  //Serial.println((micros() - block_start) / 1000000.0);
  measured_sample_rate = 1000000.0 / (float(micros() - block_start) / block_size);

  magnitude = sqrt(sq(Q1) + sq(Q2) - Q1 * Q2 * gtzl_coeff);
  Serial.println(magnitude);
  //Serial.println(measured_sample_rate);

}
