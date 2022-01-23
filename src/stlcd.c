#include "stlcd.h"

#include <avr/io.h>
#include <util/delay.h>


#define BLA_DDR DDRB
#define BLA_PIN PINB
#define BLA_PORT PORTB
#define BLA 2

#define SID_DDR DDRB
#define SID_PIN PINB
#define SID_PORT PORTB
#define SID 1

#define SCLK_DDR DDRB
#define SCLK_PIN PINB
#define SCLK_PORT PORTB
#define SCLK 0

#define A0_DDR DDRD
#define A0_PIN PIND
#define A0_PORT PORTD
#define A0 7

#define RST_DDR DDRD
#define RST_PIN PIND
#define RST_PORT PORTD
#define RST 6

#define CS_DDR DDRD
#define CS_PIN PIND
#define CS_PORT PORTD
#define CS 5

int pagemap[] = { 7, 6, 5, 4, 3, 2, 1, 0 };

inline void spiwrite(uint8_t c) {
	int8_t i;

	for (i=7; i>=0; i--) {
		SCLK_PORT &= ~_BV(SCLK);
		if (c & _BV(i))
			SID_PORT |= _BV(SID);
		else
			SID_PORT &= ~_BV(SID);
		SCLK_PORT |= _BV(SCLK);
	}
}

void st7565_command(uint8_t c) {
  A0_PORT &= ~_BV(A0);

  spiwrite(c);
}

void st7565_init(bool reverse_color) {
  // set pin directions
  SID_DDR |= _BV(SID);
  SCLK_DDR |= _BV(SCLK);
  A0_DDR |= _BV(A0);
  RST_DDR |= _BV(RST);
  CS_DDR |= _BV(CS);
  
  // toggle RST low to reset; CS low so it'll listen to us
  CS_PORT &= ~_BV(CS);
  RST_PORT &= ~_BV(RST);
  _delay_ms(500);
  RST_PORT |= _BV(RST);

  // LCD bias select
  st7565_command(CMD_SET_BIAS_7);
  // ADC select
  st7565_command(CMD_SET_ADC_NORMAL);
  // SHL select
  st7565_command(CMD_SET_COM_NORMAL);
  // Initial display line
  st7565_command(CMD_SET_DISP_START_LINE);

  if (reverse_color) {
    st7565_command(CMD_SET_DISP_REVERSE);
  } else {
    st7565_command(CMD_SET_DISP_NORMAL);
  }

  // turn on voltage converter (VC=1, VR=0, VF=0)
  st7565_command(CMD_SET_POWER_CONTROL | 0x4);
  // wait for 50% rising
  _delay_ms(50);

  // turn on voltage regulator (VC=1, VR=1, VF=0)
  st7565_command(CMD_SET_POWER_CONTROL | 0x6);
  // wait >=50ms
  _delay_ms(50);

  // turn on voltage follower (VC=1, VR=1, VF=1)
  st7565_command(CMD_SET_POWER_CONTROL | 0x7);
  // wait
  _delay_ms(10);

  // set lcd operating voltage (regulator resistor, ref voltage resistor)
  st7565_command(CMD_SET_RESISTOR_RATIO | 0x6);

  // initial display line
  // set page address
  // set column address
  // write display data
}

void st7565_data(uint8_t c) {
  A0_PORT |= _BV(A0);

  spiwrite(c);
}

void st7565_set_brightness(uint8_t val) {
	st7565_command(CMD_SET_VOLUME_FIRST);
	st7565_command(CMD_SET_VOLUME_SECOND | (val & 0x3f));
}

void clear_screen(void) {
  uint8_t p, c;
  
  for(p = 0; p < 8; p++) {
    /*
      putstring("new page! ");
      uart_putw_dec(p);
      putstring_nl("");
    */

    st7565_command(CMD_SET_PAGE | p);
    for(c = 0; c < 129; c++) {
      //uart_putw_dec(c);
      //uart_putchar(' ');
      st7565_command(CMD_SET_COLUMN_LOWER | (c & 0xf));
      st7565_command(CMD_SET_COLUMN_UPPER | ((c >> 4) & 0xf));
      st7565_data(0x0);
    }     
  }
}

void write_buffer(uint8_t *buffer) {
  uint8_t c, p;

  for(p = 0; p < 8; p++) {
    /*
      putstring("new page! ");
      uart_putw_dec(p);
      putstring_nl("");
    */

    st7565_command(CMD_SET_PAGE | pagemap[p]);
    st7565_command(CMD_SET_COLUMN_LOWER | (0x0 & 0xf));
    st7565_command(CMD_SET_COLUMN_UPPER | ((0x0 >> 4) & 0xf));
    st7565_command(CMD_RMW);
    st7565_data(0xff);
    
    //st7565_data(0x80);
    //continue;
    
    for(c = 0; c < 128; c++) {
      //uart_putw_dec(c);
      //uart_putchar(' ');
      st7565_data(buffer[(128*p)+c]);
    }
  }
}
