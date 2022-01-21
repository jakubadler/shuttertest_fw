#include "measuring.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>

#include <stdbool.h>

#define IS_CLOSED(x) (PINC & _BV(x))
#define IS_OPEN(x) (!IS_CLOSED(x))
#define SENSOR_CHANGED(x) ((sensor_state & _BV(x)) ^ (PINC & _BV(x)))
#define IS_ACTIVE(x) (counter_active & _BV(x))
#define SET_ACTIVE(x) (counter_active |= _BV(x))
#define SET_INACTIVE(x) (counter_active &= ~_BV(x))

enum counter
{
	MAIN,
	CENTER_OPEN,
	CENTER_CLOSE,
	END_OPEN,
	END_CLOSE
};

static uint8_t sensor_state = 0;
static uint8_t counter_active = 0;

static bool measuring = false;

static uint32_t main_timer_value = 0;
static uint32_t center_timer_open_value = 0;
static uint32_t center_timer_close_value = 0;
static uint32_t end_timer_open_value = 0;
static uint32_t end_timer_close_value = 0;

void get_measurements(struct display_data *data)
{
	data->measuring = measuring;

	if (!measuring) { // Only update data when measuring is done.
		float begin_time = main_timer_value;
		float center_time = main_timer_value - center_timer_open_value + center_timer_close_value;
		float end_time = main_timer_value - end_timer_open_value + end_timer_close_value;

		data->time1 = (1024.0f * begin_time) / F_CPU;
		data->time2 = (1024.0f * center_time) / F_CPU;
		data->time3 = (1024.0f * end_time) / F_CPU;
	}
}

ISR(PCINT1_vect)
{
	if (SENSOR_CHANGED(0) && IS_OPEN(0)) {
		// Main sensor has just opened.
		SET_ACTIVE(MAIN);

		// Reset main timer.
		main_timer_value = 0;
		TCNT1 = 0;

		// Reset center timer.
		center_timer_open_value = 0;
		TCNT0 = 0;
		SET_ACTIVE(CENTER_OPEN);

		// Reset center timer.
		TCNT2 = 0;
		end_timer_open_value = 0;
		SET_ACTIVE(END_OPEN);

		// Measuring started.
		measuring = true;
	}

	if (SENSOR_CHANGED(0) && IS_CLOSED(0)) {
		// Main sensor has just closed.
		SET_INACTIVE(MAIN);

		main_timer_value += TCNT1;

		// Reset center timer.
		center_timer_close_value = 0;
		TCNT0 = 0;
		SET_ACTIVE(CENTER_CLOSE);

		// Reset center timer.
		TCNT2 = 0;
		end_timer_close_value = 0;
		SET_ACTIVE(END_CLOSE);
	}

	if (SENSOR_CHANGED(1) && IS_OPEN(1)) {
		// Center sensor has just opened.
		center_timer_open_value += TCNT0;
		SET_INACTIVE(CENTER_OPEN);
	}

	if (SENSOR_CHANGED(1) && IS_CLOSED(1)) {
		// Center sensor has just closed.
		center_timer_close_value += TCNT0;
		SET_INACTIVE(CENTER_CLOSE);
	}

	if (SENSOR_CHANGED(2) && IS_OPEN(2)) {
		// End sensor has just opened.
		end_timer_open_value += TCNT2;
		SET_INACTIVE(END_OPEN);
	}

	if (SENSOR_CHANGED(2) && IS_CLOSED(2)) {
		// End sensor has just closed.
		end_timer_close_value += TCNT2;
		SET_INACTIVE(END_CLOSE);

		// Measuring is done.
		measuring = false;
	}

	sensor_state = PINC & 0x07;
}

ISR(TIMER1_OVF_vect)
{
	// Main timer overflow.

	if (IS_ACTIVE(MAIN)) {
		main_timer_value += 0xffff;
	}
}

ISR(TIMER0_OVF_vect)
{
	// Center timer overflow.

	if (IS_ACTIVE(CENTER_OPEN)) {
		center_timer_open_value += 0xff;
	}

	if (IS_ACTIVE(CENTER_CLOSE)) {
		center_timer_close_value += 0xff;
	}
}

ISR(TIMER2_OVF_vect)
{
	// End timer overflow.

	if (IS_ACTIVE(END_OPEN)) {
		end_timer_open_value += 0xff;
	}

	if (IS_ACTIVE(END_CLOSE)) {
		end_timer_close_value += 0xff;
	}
}

void measuring_init(void)
{
	DDRC = 0x00;

	sensor_state = PINC & 0x07;

	// Configure interrupts.
	PCICR |= _BV(PCIE1);
	PCMSK1 = _BV(PCINT8) | _BV(PCINT9) | _BV(PCINT10);

	// Initialize main timer.
	TCCR1A = 0x00;
	TCCR1B = 0x00;
	TCCR1B = _BV(CS10) | _BV(CS12); // Set frequence divider to 1024.
	TIMSK1 = _BV(TOIE2); // Interrupt on overflow.

	// Initialize center timer.
	TCCR0A = 0x00;
	TCCR0B = 0x00;
	TCCR0B = _BV(CS10) | _BV(CS12); // Set frequence divider to 1024.
	TIMSK0 = _BV(TOIE2); // Interrupt on overflow.

	// Initialize end timer.
	TCCR2A = 0x00;
	TCCR2B = 0x00;
	TCCR2B = _BV(CS10) | _BV(CS12); // Set frequence divider to 1024.
	TIMSK2 = _BV(TOIE2); // Interrupt on overflow.

	sei();
}

