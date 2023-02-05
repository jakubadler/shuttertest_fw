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

static struct display_data dd;

void setup(void)
{
	display_init();
	display_update(&dd);

	measuring_init();

	LED_DDR |= _BV(LED);

}

int main(void)
{
	setup();

	while (1) {
		get_measurements(&dd);

		SET_LED(dd.measuring);
		display_update(&dd);
		_delay_ms(1);

	}

	return 0;
}

