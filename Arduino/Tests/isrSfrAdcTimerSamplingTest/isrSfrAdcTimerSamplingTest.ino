const uint16_t sample_rate = 6000; // Hz
const uint16_t sample_duration = 1; // S

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

// analog input wired to [Arduino] Pin A0 -> [ATmega328P-PU] Port C pin 0 (PC0)
const uint16_t analog_pin = PC0;

// precomputed constants for the lowest frequencies Timer/Counter1 can yield with each prescaler tap
// calculated based on 16 MHz system clock using min_freq = ((2^-timer_bitlength)*sys_freq)/prescaler
const uint16_t t1_prescaler_taps[]       = {1,   8,  64, 256, 1024};
const uint16_t t1_min_prescaler_freqs[] = {245, 31, 4,  1,   1};
uint8_t t1_prescaler_tap_index; // <- for storing which tap from the list was chosen
uint16_t t1_prescaler_val; // <- for storing the actual prescaler value that was chosen

// placeholder variable for value to be stored in OCR1A, calculated once prescaler is chosen in setup
uint16_t t1_compare_val;

// variables for recording the performance in a given sample period
uint32_t start_time; // uS
uint32_t end_time; // uS
volatile uint32_t num_adc_conversions = 0;

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

  Serial.begin(9600);
  
  Serial.print("Target sample rate: ");
  Serial.print(sample_rate);
  Serial.println(" Hz");
  
  Serial.print("Sample duration: ");
  Serial.print(sample_duration);
  Serial.println("s");
  
  Serial.print("Timer/Counter1 prescaler: ");
  Serial.println(t1_prescaler_val);  
  
}

void loop() {

  Serial.println("Collecting sample...");
  num_adc_conversions = 0;
  start_time = micros();
  end_time = micros();
  // start Timer/Counter1 clock with appropriate prescaler value
  TCCR1B |= (t1_prescaler_tap_index + 1);
  while (end_time - start_time < sample_duration * 1000000) {
    end_time = micros();
  }
  // pause Timer/Counter1 by clearing Clock Select bits
  TCCR1B &= ~(B111);
  Serial.print("Sample duration: ");
  Serial.print(end_time - start_time);
  Serial.println("uS");
  Serial.print("ADC conversions: ");
  Serial.println(num_adc_conversions);

  delay(sample_duration * 1000);
  
}

// ISR that will be triggered upon each completed timer period
ISR (TIMER1_COMPA_vect) {
  
  // start an ADC conversion
  ADCSRA |= (1 << ADSC);
  
}

// ISR that will be triggered upon each completed ADC conversion
ISR (ADC_vect) {

  // increment tally of ADC conversions
  ++num_adc_conversions;
  
}
