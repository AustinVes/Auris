// RELEVANT REGISTERS:
// - DDRx: Port x Data Direction Register
//    - [n] DDxn: controls whether Port x pin n (Pxn) is configured as input(0) or output(1)
// - PORTx: Port x Data Register
//    - [n] PORTxn: For Port x pin n (Pxn), where that pin is configured as...
//                          input: controls whether pin has pull-up resistor disabled(0) or enabled(1)
//                          output: controls whether pin is set to LOW(0) or HIGH(1)
// - EICRA: External Interrupt Control Register A
//    - [2n+1:2n] ISCn1:0: controls what signal on External Interrupt n will trigger an INTn interrupt
// - EIMSK: External Interrupt Mask Register
//    - [n] INTn: controls whether INTn interrupts are disabled(0) or enabled(1)


// button wired to [Arduino] Pin 3 -> [ATmega328P-PU] Port D pin 3 (PD3)
//                                 -> [ATmega328P-PU] External Interrupt Request 1 pin (INT1)
const int button_pin = PD3;
// LED wired to [Arduino] Pin 13 -> [ATmega328P-PU] Port B pin 5 (PB5)
const int led_pin = PB5;

// implementing a minimum time between interrupts before the ISR will run again helps reduce bounce
volatile unsigned int current_interrupt_time; // ms
volatile unsigned int last_interrupt_time = 0; // ms
const unsigned int min_wait = 50; // ms

void setup() {

  // configure LED pin as output
  DDRB |= (1 << led_pin);
  // set LED pin to LOW
  PORTB &= ~(1 << led_pin);

  // configure button pin as input
  DDRB &= ~(1 << button_pin);
  // disable pull-up resistors on button pin
  PORTB &= ~(1 << button_pin);

  // configure INT1 interrupt requests to be generated on rising edge of button pin (PD3/INT1)
  EICRA &= ~(1 << ISC11);
  EICRA |= (1 << ISC10);
  // enable INT1 interrupts
  EIMSK |= (1 << INT1);
  // enable global interrupts
  sei();

}

void loop() {

  // simulate important main loop stuff
  delay(1000);

}

ISR (INT1_vect) {

  current_interrupt_time = millis();
  // only continue ISR if min_time has elapsed between this interrupt and the last successful one
  if (current_interrupt_time - last_interrupt_time >= min_wait) {
    last_interrupt_time = current_interrupt_time;
    // toggle LED
    PORTB ^= (1 << led_pin);
  }
  
}
