#ifndef SHUTTER_TEST__IO__H__
#define SHUTTER_TEST__IO__H__

/**
 * PORT D
 * LED, switch, LCD
 */
#define LED_DDR DDRD
#define LED_PORT PORTD
#define LED 0

#define SWITCH_DDR DDRD
#define SWITCH_PORT PORTD
#define SWITCH_PIN PIND
#define SWITCH 0

// LCD pins
#define SID_DDR DDRD
#define SID_PIN PIND
#define SID_PORT PORTD
#define SID 3

#define SCLK_DDR DDRD
#define SCLK_PIN PIND
#define SCLK_PORT PORTD
#define SCLK 4

#define CS_DDR DDRD
#define CS_PIN PIND
#define CS_PORT PORTD
#define CS 5

#define RST_DDR DDRD
#define RST_PIN PIND
#define RST_PORT PORTD
#define RST 6

#define A0_DDR DDRD
#define A0_PIN PIND
#define A0_PORT PORTD
#define A0 7


#endif
