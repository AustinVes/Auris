// RELEVANT REGISTERS:
// - DDRx: Port x Data Direction Register
//    - [n] DDxn: controls whether Port x pin n (Pxn) is configured as input(0) or output(1)
// - PORTx: Port x Data Register
//    - [n] PORTxn: For Port x pin n (Pxn), where that pin is configured as...
//                          input: controls whether pin has pull-up resistor disabled(0) or enabled(1)
//                          output: controls whether pin is set to LOW(0) or HIGH(1)
// - PRR: Power Reduction Register
//    - [0] PRADC: controls whether ADC is enabled(0) or disabled(1)
//    - [3] PRTIM1: controls whether Timer/Counter1 is enabled (0) or disabled (1)
// - TCCR1A: Timer/Counter1 Control Register A
//    - [1:0] WGM11:0 (+ WGM13:2 in TCCR1B): controls the waveform generation mode
//    - [7:6] COM1A1:0: controls the compare output mode for channel A on Timer/Counter1
// - TCCR1B: Timer/Counter1 Control Register B
//    - [2:0] CS12:0: controls the clock source for Timer/Counter1
//    - [4:3] WGM13:2 (+ WGM11:0 in TCCR1A): controls the waveform generation mode
// - TCNT1: Timer/Counter1
//    - [15:8] TCNT1H: the high byte of the counter value
//    - [7:0] TCNT1L: the low byte of the counter value
// - OCR1A: Output Compare Register 1 A
//    - [15:8] OCR1AH: the high byte of the value against which the counter value should be compared
//    - [7:0] OCR1AL: the low byte of the value against which the counter value should be compared
// - TIMSK1: Timer/Counter 1 Interrupt Mask Register
//    - [1] OCIE1A: controls whether TIMER1_COMPA interrupts are disabled(0) or enabled(1)
// - ADCSRA: ADC Control and Status Register A
//    - [2:0] ADPS2:0: sets the prescaler value for the ADC clock
//    - [3] ADIE: controls whether ADC interrupts are disabled(0) or enabled(1)
//    - [6] ADSC: triggers a conversion when set, clears when conversion is complete
//    - [7] ADEN: controls whether ADC is disabled(0) or enabled(1)
// - ADMUX: ADC Multiplexer Selection Register
//    - [3:0] MUX3:0: controls which ADC pin will be read for conversion
//    - [5] ADLAR: controls whether values in the ADC Data Register are right(0) or left(1) adjusted
//    - [7:6] REFS1:0: controls the analog reference voltage
// - DIDR0: Digital Input Disable Register 0
//    - [5:0] ADCnD: controls whether digital input buffer on ADC pin n is enabled(0) or disabled(1)
// - ADCW: ADC Data Register
//    - [7:0] ADCL: the low byte of the result of the latest conversion
//    - [15:8] ADCH: the high byte of the result of the latest conversion

// based on the optimized Goertzel algorithm described here: https://www.embedded.com/design/configurable-systems/4024443/The-Goertzel-Algorithm

// analog input wired to [Arduino] Pin A0 -> [ATmega328P-PU] Port C pin 0 (PC0)
const uint16_t audio_pin = PC0;

// set parameters for Goertzel algorithm
const uint16_t target_freq = 400; // Hz
const uint16_t sample_rate = 1200; // Hz
//const uint16_t block_size = 120; // <-- bin width = 10Hz, sample time = 0.1s
const uint16_t block_size = 300; // <-- bin width = 4Hz, sample time = 0.25s // OOPS THIS IS centered at 180 HZ

// calculate precomputed constants from parameters
const int16_t gtzl_k = 0.5 + ((int32_t)block_size * target_freq / sample_rate);
const double gtzl_w = (TWO_PI / block_size) * gtzl_k;
const double gtzl_cosine = cos(gtzl_w);
const double gtzl_coeff = 2 * gtzl_cosine;

// variables to store per-sample processing results
volatile double Q0, Q1, Q2;
volatile uint32_t num_adc_conversions;

// variable to store measured magnitude (squared) of target frequency
double gtzl_magnitude;

// precomputed constants for the lowest frequencies Timer/Counter1 can yield with each prescaler tap
// calculated based on 16 MHz system clock using min_freq = ((2^-timer_bitlength)*sys_freq)/prescaler
const uint16_t t1_prescaler_taps[]      = {1,   8,  64, 256, 1024};
const uint16_t t1_min_prescaler_freqs[] = {245, 31, 4,  1,   1};
uint8_t t1_prescaler_tap_index; // <- for storing which tap from the list was chosen
uint16_t t1_prescaler_val; // <- for storing the actual prescaler value that was chosen

// placeholder variable for value to be stored in OCR1A, calculated once prescaler is chosen in setup
uint16_t t1_compare_val;

void setup() {

  // select the lowest possible Timer/Counter1 prescaler value given target sample rate
  for (int i=0; i < sizeof(t1_prescaler_taps)/sizeof(t1_prescaler_taps[0]); ++i) {
    if (t1_min_prescaler_freqs[i] <= sample_rate) {
      t1_prescaler_tap_index = i;
      t1_prescaler_val = t1_prescaler_taps[i];
      break;
    }
  }

  // calculate value to be stored in OCR1A based on Timer/Counter1 prescaler value
  t1_compare_val = 16000000/(t1_prescaler_val * sample_rate);

  // pause Timer/Counter1 by clearing Clock Select bits
  TCCR1B &= ~(B111);

  // ensure Timer/Counter1 and ADC aren't already disabled externally to conserve power
  PRR &= ~(1 << PRADC);
  PRR &= ~(1 << PRTIM1);

  // disable ADC
  ADCSRA &= ~(1 << ADEN);
  // configure ADC to read from ADC0 input channel (A0)
  ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));
  // configure analog reference voltage as AVcc w/ ext. capacitor at AREF pin (provided by Arduino)
  ADMUX &= ~(1 << REFS1);
  ADMUX |= (1 << REFS0);
  // configure conversion results to be stored right-adjusted in ADCW
  ADMUX &= ~(1 << ADLAR);
  // set ADC clock prescaler value to 128 (default)
  ADCSRA |= ((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));
  // disable digital input buffers for all ADC channels to conserve power
  DIDR0 |= ~0;
  // temporarily disable ADC interrupts for throwaway conversion
  ADCSRA &= ~(1 << ADIE);
  // re-enable ADC
  ADCSRA |= (1 << ADEN);
  // perform throwaway conversion (the first conversion after re-enabling ADC takes longer than usual)
  ADCSRA |= (1 << ADSC);
  while ((ADCSRA & (1 << ADSC)) >> ADSC);
  // re-enable ADC interrupts to be triggered on every completed conversion
  ADCSRA |= (1 << ADIE);

  // reset Timer/Counter1 control settings;
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1C = 0;
  // set Waveform Generation Mode for Timer/Counter1 to Clear Timer on Compare Match (CTC)
  TCCR1B |= (1 << WGM12);
  // reset Timer/Counter1 counter value
  TCNT1 = 0;
  // set the Channel A value against which the Timer/Counter1 counter value should be compared
  OCR1A = t1_compare_val;
  // enable TIMER1_COMPA interrupts to be triggered on compare matches in Channel A of Timer/Counter1
  TIMSK1 |= (1 << OCIE1A);

   // enable global interrupts
  sei();

  Serial.begin(115200);
  while (!Serial) {}
  
  Serial.println("Serial connection initiated.\n");

  Serial.print("Target frequency: ");
  Serial.print(target_freq);
  Serial.println(" Hz");

  Serial.print("Sampling rate: ");
  Serial.print(sample_rate);
  Serial.println(" Hz");

  Serial.print("Timer/Counter1 prescaler: ");
  Serial.println(t1_prescaler_val);

  Serial.print("Timer/Counter1 OCR1A value: ");
  Serial.println(t1_compare_val);

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

  // reset variables for new sampling block
  gtzl_magnitude = num_adc_conversions = Q0 = Q1 = Q2 = 0;

  // start Timer/Counter1 clock with appropriate prescaler value
  TCCR1B |= (t1_prescaler_tap_index + 1);
  // wait until the sampling block is complete
  while (num_adc_conversions < block_size);
  // pause Timer/Counter1 by clearing Clock Select bits
  TCCR1B &= ~(B111);

  gtzl_magnitude = sqrt(sq(Q1) + sq(Q2) - Q1 * Q2 * gtzl_coeff);
  Serial.println(gtzl_magnitude);
}

// ISR that will be triggered upon each completed timer period
ISR (TIMER1_COMPA_vect) {
  
  // start an ADC conversion
  ADCSRA |= (1 << ADSC);
  
}

// ISR that will be triggered upon each completed ADC conversion
ISR (ADC_vect) {

  // perform Goertzel algorithm per-sample processing
  Q0 = gtzl_coeff * Q1 - Q2 + ((int16_t)ADCW - 512);
  Q2 = Q1;
  Q1 = Q0;
  ++num_adc_conversions;

}
