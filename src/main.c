#include "display.h"
#include "measuring.h"

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/atomic.h>
#include <util/delay.h>

#include <stdint.h>
#include <stdbool.h>

#include "io.h"

#define LED_ON() (LED_PORT |= _BV(LED))
#define LED_OFF() (LED_PORT &= ~_BV(LED))
#define SET_LED(x) (!!(x) ? LED_ON() : LED_OFF())

static bool swstate = false;
static uint8_t mode = MODE_HORIZ;

static struct meas_data mdata;

void setup(void)
{
	// Enable INT0, that's the switch button.
	EICRA = 0x01;
	EIMSK |= _BV(INT0);

	// Turn off unused stuff.
	ADCSRA = 0;
	power_all_disable();
	power_timer1_enable();
	MCUCR = _BV(BODS) | _BV(BODSE);
	MCUCR = _BV(BODS);

	ENABLE_DDR = 0xff; // output
	ENABLE_PORT = 0x00;
	
	display_init();

	ENABLE_PORT |= _BV(ENABLE_BACKLIGHT);
	measuring_init();
	measuring_init_mode(mode);

	LED_DDR |= _BV(LED);
	SWITCH_DDR &= ~_BV(SWITCH);
	SWITCH_PORT |= _BV(SWITCH); // Enable internal pull-up resistor.

	_delay_ms(10);

	swstate = !(SWITCH_PIN & _BV(SWITCH));
}

ISR(INT0_vect)
{
	sleep_disable();
}

int main(void)
{
	setup();

	while (1) {
		bool swstate_new = !(SWITCH_PIN & _BV(SWITCH));
		if (swstate && !swstate_new) {
			mode++;
			mode %= MODE_LAST;
			measuring_init_mode(mode);
		}
		swstate = swstate_new;

		get_measurements(&mdata);

		SET_LED(mdata.measuring || swstate);

		display_update(&mdata, mode);
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_mode();
	}

	return 0;
}

