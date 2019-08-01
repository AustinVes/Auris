// RELEVANT REGISTERS:
// - DDRx: Port x Data Direction Register
//    - [n] DDxn: controls whether Port x pin n (Pxn) is configured as input(0) or output(1)
// - PORTx: Port x Data Register
//    - [n] PORTxn: For Port x pin n (Pxn), where that pin is configured as...
//                          input: controls whether pin has pull-up resistor disabled(0) or enabled(1)
//                          output: controls whether pin is set to LOW(0) or HIGH(1)
// - PINx: Port x Input Pins Address
//    - [n] PINxn: contains the digital input value of Port x pin n
// - EICRA: External Interrupt Control Register A
//    - [2n+1:2n] ISCn1:0: controls what signal on External Interrupt n will trigger an INTn interrupt
// - EIMSK: External Interrupt Mask Register
//    - [n] INTn: controls whether INTn interrupts are disabled(0) or enabled(1)
// - PRR: Power Reduction Register
//    - [3] PRADC: controls whether ADC is enabled(0) or disabled(1)
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


// button wired to [Arduino] Pin 3 -> [ATmega328P-PU] Port D pin 3 (PD3)
//                                 -> [ATmega328P-PU] External Interrupt Request 1 pin (INT1)
const uint16_t button_pin = PD3;

// analog input wired to [Arduino] Pin A0 -> [ATmega328P-PU] Port C pin 0 (PC0)
const uint16_t analog_pin = PC0;

// implementing a minimum time between interrupts before the ISR will run again helps reduce bounce
volatile uint16_t current_interrupt_time; // ms
volatile uint16_t last_interrupt_time = 0; // ms
const uint16_t min_wait = 50; // ms
// adding a comparator with hysteresis between the button and the input pin will reduce trigger bounce
// as a workaround, change detection has less bounce than rise or fall detection, so use it + a toggle
volatile uint8_t ignore_button_change = 0;

// variables for breaking the ADC readings out of the ISR to send them over Serial
volatile uint8_t new_ADC_conversion = 0;
volatile uint16_t latest_conversion_value;

void setup() {

  // configure button pin as input
  DDRB &= ~(1 << button_pin);
  // disable pull-up resistors on button pin
  PORTB &= ~(1 << button_pin);
  // configure INT1 interrupt requests to be generated on logical change of button pin (PD3/INT1)
  EICRA &= ~(1 << ISC11);
  EICRA |= (1 << ISC10);
  // enable INT1 interrupts
  EIMSK |= (1 << INT1);

  // ensure ADC isn't already disabled externally to conserve power
  PRR &= ~(1 << PRADC);
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
  
  // enable global interrupts
  sei();

  Serial.begin(9600);
}

void loop() {

  if (new_ADC_conversion) {
    Serial.println(ADCW);
    new_ADC_conversion = 0;
  }

}

// ISR that will be triggered upon each change in voltage on the button pin
ISR (INT1_vect) {

  current_interrupt_time = millis();
  // only continue ISR if min_time has elapsed between this interrupt and the last successful one
  if (current_interrupt_time - last_interrupt_time >= min_wait) {
    last_interrupt_time = current_interrupt_time;
    
    //only continue on every other successful interrupt
    if (!ignore_button_change) {
      // start an ADC conversion
      ADCSRA |= (1 << ADSC);
    }
    ignore_button_change = !ignore_button_change;
  }
  
}

// ISR that will be triggered upon each completed ADC conversion
ISR (ADC_vect) {

  // copy ADC reading out of ISR
  latest_conversion_value = ADCW;
  new_ADC_conversion = 1;
  
}
