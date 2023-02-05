#include "display.h"
#include "measuring.h"

#include <avr/io.h>
#include <avr/interrupt.h>
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

	display_init();
	measuring_init();
	measuring_init_mode(mode);

	LED_DDR |= _BV(LED);
	SWITCH_DDR &= ~_BV(SWITCH);
	SWITCH_PORT |= _BV(SWITCH); // Enable internal pull-up resistor.

	_delay_ms(10);

	swstate = !(SWITCH_PIN & _BV(SWITCH));
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

	}

	return 0;
}

