// RELEVANT REGISTERS:
// - DDRx: Port x Data Direction Register
//    - [n] DDxn: controls whether Port x pin n (Pxn) is configured as input(0) or output(1)
// - PORTx: Port x Data Register
//    - [n] PORTxn: For Port x pin n (Pxn), where that pin is configured as...
//                          input: controls whether pin has pull-up resistor disabled(0) or enabled(1)
//                          output: controls whether pin is set to LOW(0) or HIGH(1)
// - PRR: Power Reduction Register
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


// LED wired to [Arduino] Pin 13 -> [ATmega328P-PU] Port B pin 5 (PB5)
const int led_pin = PB5;

// value to be stored in OCR1A, calculated for 0.5s delay using 16MHz clock source w/ prescaler of 256
const uint16_t t1_compare_val = 31250;

void setup() {

  // configure LED pin as output
  DDRB |= (1 << led_pin);

  // pause Timer/Counter1 by clearing Clock Select bits
  TCCR1B &= ~(B111);
  // ensure Timer/Counter1 isn't disabled
  PRR &= ~(1 << PRTIM1);
  // reset Timer/Counter1 control settings;
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1C = 0;
  // set Waveform Generation Mode for Timer/Counter1 to Clear Timer on Compare Match (CTC)
  TCCR1B |= (1 << WGM12);
  // reset Timer/Counter1 counter value
  TCNT1 = 0;
  // set the Channel A value against which the Timer/Counter1 counter value should be compared
  OCR1A = t1_comp;
  // enable TIMER1_COMPA interrupts to be triggered on compare matches in Channel A of Timer/Counter1
  TIMSK1 |= (1 << OCIE1A);
  // enable global interrupts
  sei();
  // restart Timer/Counter1 clock with a prescaler value of 256
  TCCR1B |= (1 << CS12);
}

void loop() {

  // simulate important main loop stuff
  delay(1000);

}

ISR (TIMER1_COMPA_vect) {

  // toggle LED
  PORTB ^= (1 << led_pin);
  
}
