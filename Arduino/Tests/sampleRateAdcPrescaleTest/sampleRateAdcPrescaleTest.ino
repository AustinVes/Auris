int analogPin = A0;
int numSamples = 10000;

// Secret sauce to change ADC prescale from 128 to 16
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

void setup() {

  Serial.begin(9600);
  while (!Serial) {} // wait for serial connection
  Serial.println("Beginning test...\n");

  int start_time;
  int end_time;
  int elapsed_time;
  long sample_rate;

  // default ADC prescale (128) test
  start_time = millis();
  for (int i = 0; i < numSamples; i++) {
    analogRead(analogPin);
  }
  end_time = millis();
  elapsed_time = end_time - start_time;
  sample_rate = long(float(numSamples) / (float(elapsed_time)/1000.0));

  Serial.println("ADC prescale: 128");
  Serial.print(numSamples);
  Serial.print(" samples taken in ");
  Serial.print(elapsed_time);
  Serial.println(" ms");
  Serial.print("Avg sample rate = ");
  Serial.print(sample_rate);
  Serial.println(" Hz");
  Serial.println("\n");

  // change ADC prescale to 16
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);

  // modified ADC prescale (16) test
  start_time = millis();
  for (int i = 0; i < numSamples; i++) {
    analogRead(analogPin);
  }
  end_time = millis();
  elapsed_time = end_time - start_time;
  sample_rate = long(float(numSamples) / (float(elapsed_time)/1000.0));

  Serial.println("ADC prescale: 16");
  Serial.print(numSamples);
  Serial.print(" samples taken in ");
  Serial.print(elapsed_time);
  Serial.println(" ms");
  Serial.print("Avg sample rate = ");
  Serial.print(sample_rate);
  Serial.println(" Hz");
  Serial.println("\n");

}

void loop() {

}
