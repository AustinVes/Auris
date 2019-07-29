// the purpose of this test is to interact with special function registers with C at the barest level
// I avoid using built-in register macros here for learning purposes (at the cost of readability)

// Arduino comes with header files for the ATmega328p that define each register's name as a macro
// by which you can access its value
// they work like this:
// #define ADCL _SFR_MEM8(mm_addr)
// #define _SFR_MEM8(mm_addr) _MMIO_BYTE(mm_addr)
// #define _MMIO_BYTE(mm_addr) (*(volatile uint8_t *)(mm_addr))
// there are other variations of this behind some of the register-name macros
// (e.g. for 16-bit registers, registers that deal with I/O, etc.)

// relevant registers:
// - DDRx: controls whether pins are input (0) or output (1) for Port x
// - PORTx: controls whether pins set to output are LOW (0) or HIGH (1)
//          controls whether pins set to input have pull-up resistor disabled (0) or enabled (1)
//          for Port x
// - PINx: reads input values from pins on Port x

// LED wired to [Arduino] Pin 13 -> [ATmega328P-PU] Port B pin 5 (PB5) -> [SRAM] DDRB:5 (0x24:5)
//                                                                     -> [SRAM] PORTB:5 (0x25:5)

// button wired to [Arduino] Pin 3 -> [ATmega328P-PU] Port D pin 3 (PD3) -> [SRAM] DDRD:3 (0x2A:3)
//                                                                       -> [SRAM] PORTD:3 (0x2B:3)
//                                                                       -> [SRAM] PIND:3 (0x29:3)

void setup() {

  // set [Arduino] Pin 13 to output
  (*(volatile uint8_t *)(0x24)) |= (1 << 5);

  // set [Arduino] Pin 3 to input
  (*(volatile uint8_t *)(0x2A)) |= (1 << 3);
  // disable pull-up resistors on [Arduino] Pin 3
  (*(volatile uint8_t *)(0x2B)) &= ~(1 << 3);

}

void loop() {

  // check if Pin 3 reads HIGH
  if (((*(volatile uint8_t *)(0x29))&(1<<3)) >> 3) {
    // set [Arduino] Pin 13 to HIGH
    (*(volatile uint8_t *)(0x25)) |= (1 << 5);
  } else {
    // set [Arduino] Pin 13 to LOW
    (*(volatile uint8_t *)(0x25)) &= ~(1 << 5);
  }

}
