// based on the optimized Goertzel algorithm described here: https://www.embedded.com/design/configurable-systems/4024443/The-Goertzel-Algorithm

const int analog_pin = A0;

const unsigned int goertzel_N = 10000; // very high number of samples
const unsigned int goertzel_k = 301; // stand-in k, not actually calculated
const float goertzel_w = (2 * TWO_PI / goertzel_N)* goertzel_k;
const float goertzel_cosine = cos(goertzel_w);
const float goertzel_sine = sin(goertzel_w);
const float goertzel_coeff_float = 2 * goertzel_cosine;
const int goertzel_coeff_int = goertzel_coeff_float;

int sample;

float goertzel_Q0_float;
float goertzel_Q1_float;
float goertzel_Q2_float;

int goertzel_Q0_int;
int goertzel_Q1_int;
int goertzel_Q2_int;

// Secret sauce to change ADC prescale from 128 to 16
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

void setup() {

  Serial.begin(9600);
  while (!Serial) {}
  Serial.println("Serial connection initiated.\n");

  int start_time;
  int end_time;
  int elapsed_time;
  long sample_rate;

  // DEFAULT ADC PRESCALE (128) TESTS
  Serial.println("ADC prescaler division factor set to 128 bits.");

  // float math test
  Serial.println("Beginning test using floating point math...");
  goertzel_Q0_float = 0.0;
  goertzel_Q1_float = 0.0;
  goertzel_Q2_float = 0.0;

  start_time = millis();
  for (int i = 0; i < goertzel_N; i++) {
    sample = analogRead(analog_pin) - 512;
    goertzel_Q0_float = goertzel_coeff_float * goertzel_Q1_float - goertzel_Q2_float + sample;
    goertzel_Q2_float = goertzel_Q1_float;
    goertzel_Q1_float = goertzel_Q0_float;
  }
  end_time = millis();
  elapsed_time = end_time - start_time;
  sample_rate = int(float(goertzel_N) / (float(elapsed_time)/1000.0));
  Serial.println("Test complete.");
  Serial.print("Average sample rate: ");
  Serial.print(sample_rate);
  Serial.println(" Hz\n");

  // int math test
  Serial.println("Beginning test using integer math...");
  goertzel_Q0_int = 0;
  goertzel_Q1_int = 0;
  goertzel_Q2_int = 0;

  start_time = millis();
  for (int i = 0; i < goertzel_N; i++) {
    sample = analogRead(analog_pin) - 512;
    goertzel_Q0_int = goertzel_coeff_int * goertzel_Q1_int - goertzel_Q2_int + sample;
    goertzel_Q2_int = goertzel_Q1_int;
    goertzel_Q1_int = goertzel_Q0_int;
  }
  end_time = millis();
  elapsed_time = end_time - start_time;
  sample_rate = int(float(goertzel_N) / (float(elapsed_time)/1000.0));
  Serial.println("Test complete.");
  Serial.print("Average sample rate: ");
  Serial.print(sample_rate);
  Serial.println(" Hz\n");

  // float to int math error test
  Serial.println("Beginning test for error between float and int math...");
  for (int s_n = 0; s_n < 3; s_n++) {
    int sample_n = pow(10, s_n+2)+1;
    Serial.print("Sample size: ");
    Serial.println(sample_n);

    for (int round_num = 1; round_num <= 5; round_num++) {
      Serial.print("           Round ");
      Serial.println(round_num);

      goertzel_Q0_float = 0.0;
      goertzel_Q1_float = 0.0;
      goertzel_Q2_float = 0.0;

      goertzel_Q0_int = 0;
      goertzel_Q1_int = 0;
      goertzel_Q2_int = 0;

      for (int i = 0; i < sample_n; i++) {
        sample = analogRead(analog_pin) - 512;

        goertzel_Q0_float = goertzel_coeff_float * goertzel_Q1_float - goertzel_Q2_float + sample;
        goertzel_Q2_float = goertzel_Q1_float;
        goertzel_Q1_float = goertzel_Q0_float;

        goertzel_Q0_int = goertzel_coeff_int * goertzel_Q1_int - goertzel_Q2_int + sample;
        goertzel_Q2_int = goertzel_Q1_int;
        goertzel_Q1_int = goertzel_Q0_int;
      }

      float goertzel_magnitude_float = sqrt(sq(goertzel_Q1_float) + sq(goertzel_Q2_float) - goertzel_Q1_float * goertzel_Q2_float * goertzel_coeff_float);
      float goertzel_magnitude_int = sqrt(sq(goertzel_Q1_int) + sq(goertzel_Q2_int) - goertzel_Q1_int * goertzel_Q2_int * goertzel_coeff_float);

      Serial.print("              (float)  Q1 = ");
      Serial.println(goertzel_Q1_float);
      Serial.print("                       Q2 = ");
      Serial.println(goertzel_Q2_float);
      Serial.print("                      Mag = ");
      Serial.println(goertzel_magnitude_float);
      Serial.print("              (int)    Q1 = ");
      Serial.println(goertzel_Q1_int);
      Serial.print("                       Q2 = ");
      Serial.println(goertzel_Q2_int);
      Serial.print("                      Mag = ");
      Serial.println(goertzel_magnitude_int);
    }
  }
  Serial.println("Test complete.\n");

  // SHORTENED ADC PRESCALE (16) TESTS
  Serial.println("ADC prescaler division factor set to 16 bits.");
  // change ADC prescale to 16
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
  
  // float math test
  Serial.println("Beginning test using floating point math...");
  goertzel_Q0_float = 0.0;
  goertzel_Q1_float = 0.0;
  goertzel_Q2_float = 0.0;

  start_time = millis();
  for (int i = 0; i < goertzel_N; i++) {
    sample = analogRead(analog_pin) - 512;
    goertzel_Q0_float = goertzel_coeff_float * goertzel_Q1_float - goertzel_Q2_float + sample;
    goertzel_Q2_float = goertzel_Q1_float;
    goertzel_Q1_float = goertzel_Q0_float;
  }
  end_time = millis();
  elapsed_time = end_time - start_time;
  sample_rate = int(float(goertzel_N) / (float(elapsed_time)/1000.0));
  Serial.println("Test complete.");
  Serial.print("Average sample rate: ");
  Serial.print(sample_rate);
  Serial.println(" Hz\n");

  // int math test
  Serial.println("Beginning test using integer math...");
  goertzel_Q0_int = 0;
  goertzel_Q1_int = 0;
  goertzel_Q2_int = 0;

  start_time = millis();
  for (int i = 0; i < goertzel_N; i++) {
    sample = analogRead(analog_pin) - 512;
    goertzel_Q0_int = goertzel_coeff_int * goertzel_Q1_int - goertzel_Q2_int + sample;
    goertzel_Q2_int = goertzel_Q1_int;
    goertzel_Q1_int = goertzel_Q0_int;
  }
  end_time = millis();
  elapsed_time = end_time - start_time;
  sample_rate = int(float(goertzel_N) / (float(elapsed_time)/1000.0));
  Serial.println("Test complete.");
  Serial.print("Average sample rate: ");
  Serial.print(sample_rate);
  Serial.println(" Hz\n");

  // float to int math error test
  Serial.println("Beginning test for error between float and int math...");
  for (int s_n = 0; s_n < 3; s_n++) {
    int sample_n = pow(10, s_n+2)+1;
    
    Serial.print("Sample size: ");
    Serial.println(sample_n);

    for (int round_num = 1; round_num <= 5; round_num++) {
      Serial.print("           Round ");
      Serial.println(round_num);

      goertzel_Q0_float = 0.0;
      goertzel_Q1_float = 0.0;
      goertzel_Q2_float = 0.0;

      goertzel_Q0_int = 0;
      goertzel_Q1_int = 0;
      goertzel_Q2_int = 0;

      for (int i = 0; i < sample_n; i++) {
        sample = analogRead(analog_pin) - 512;

        goertzel_Q0_float = goertzel_coeff_float * goertzel_Q1_float - goertzel_Q2_float + sample;
        goertzel_Q2_float = goertzel_Q1_float;
        goertzel_Q1_float = goertzel_Q0_float;

        goertzel_Q0_int = goertzel_coeff_int * goertzel_Q1_int - goertzel_Q2_int + sample;
        goertzel_Q2_int = goertzel_Q1_int;
        goertzel_Q1_int = goertzel_Q0_int;
      }

      float goertzel_magnitude_float = sqrt(sq(goertzel_Q1_float) + sq(goertzel_Q2_float) - goertzel_Q1_float * goertzel_Q2_float * goertzel_coeff_float);
      float goertzel_magnitude_int = sqrt(sq(goertzel_Q1_int) + sq(goertzel_Q2_int) - goertzel_Q1_int * goertzel_Q2_int * goertzel_coeff_float);

      Serial.print("              (float)  Q1 = ");
      Serial.println(goertzel_Q1_float);
      Serial.print("                       Q2 = ");
      Serial.println(goertzel_Q2_float);
      Serial.print("                      Mag = ");
      Serial.println(goertzel_magnitude_float);
      Serial.print("              (int)    Q1 = ");
      Serial.println(goertzel_Q1_int);
      Serial.print("                       Q2 = ");
      Serial.println(goertzel_Q2_int);
      Serial.print("                      Mag = ");
      Serial.println(goertzel_magnitude_int);
    }
  }
  Serial.println("Test complete.\n");
  
}

void loop() {

}
