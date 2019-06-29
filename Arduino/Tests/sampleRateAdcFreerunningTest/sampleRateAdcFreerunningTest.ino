// based on this blog post: http://yaab-arduino.blogspot.com/2015/02/fast-sampling-from-analog-input.html
// with additional help from: https://garretlab.web.fc2.com/en/arduino/inside/arduino/wiring_analog.c/analogRead.html

const int analog_pin = 0;
const int block_size = 10000;
int sample_buffer;
long num_samples = 0;
long start_time, elapsed_time;

void setup() {

  Serial.begin(9600);
  Serial.println("Serial connection initiated.");

  delay(2000);

  ADCSRA = 0;                      // clear ADCSRA register
  ADCSRB = 0;                      // clear ADCSRB register
  ADMUX |= (analog_pin & 0x07);    // set analog input pin
  ADMUX |= (1 << REFS0);           // set reference voltage to default (5V)
  ADMUX |= (0 << ADLAR);           // right align ADC value

  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // set ADC prescaler to 128 bits
  //ADCSRA |= (1 << ADPS2) | (1 << ADPS1); // set ADC prescaler to 64 bits
  //ADCSRA |= (1 << ADPS2) | (1 << ADPS0); // set ADC prescaler to 32 bits
  //ADCSRA |= (1 << ADPS2); // set ADC prescaler to 16 bits
  //ADCSRA |= (1 << ADPS1) | (1 << ADPS0); // set ADC prescaler 8 bits

  // set ADC to freerunning mode
  ADCSRA |= (1 << ADATE); // enable auto trigger of ADC
  ADCSRA |= (1 << ADIE);  // enable ADC interrupts
  ADCSRA |= (1 << ADEN);  // enable ADC
  ADCSRA |= (1 << ADSC);  // start ADC measurements

}

ISR(ADC_Vect) {

  sample_buffer = (ADCH << 8) | ADCL; // store AD conversion
  num_samples++;
  
}

void loop() {

  if (num_samples >= block_size){
    ADCSRA |= (0 << ADEN);  // disable ADC
    
    elapsed_time = micros() - start_time;  // calculate elapsed time

    Serial.print("Sampling frequency: ");
    Serial.print(1000000.0 / (elapsed_time / float(block_size)));
    Serial.println(" Hz");
   
    delay(2000);
    
    start_time = micros();
    num_samples = 0;
    
    ADCSRA |= (1 << ADEN);  // enable ADC
    ADCSRA |= (1 << ADSC);  // start ADC measurements
  }

}
