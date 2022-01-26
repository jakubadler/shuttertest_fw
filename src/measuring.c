#include "measuring.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>

#include <stdbool.h>

#define IS_OPEN(x) (!(PINC & _BV((x))))
#define IS_CLOSED(x) (!(IS_OPEN((x))))
#define SENSOR_CHANGED(x) ((sensor_state & _BV(x)) ^ (PINC & _BV(x)))

#define SENSOR_DISTANCE 16 // [mm]
//#define COUNTER_FREQ_DIV 1024
//#define COUNTER_FREQ_DIV 256
#define COUNTER_FREQ_DIV 8

static volatile uint8_t sensor_state = 0;

static volatile bool measuring = false;

static volatile int32_t timer_value = 0;
static volatile int32_t begin_close_time = 0;
static volatile int32_t center_open_time = 0;
static volatile int32_t center_close_time = 0;
static volatile int32_t end_open_time = 0;
static volatile int32_t end_close_time = 0;
static volatile uint8_t status = 0;

#define TIME_COEF (COUNTER_FREQ_DIV / (F_CPU / 1000000))

void get_measurements(struct display_data *data)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		data->measuring = measuring;
		data->status = status;

		if (!measuring) { // Only update data when measuring is done.

			data->time1 = begin_close_time * TIME_COEF;
			data->time2 = (center_close_time - center_open_time) * TIME_COEF;
			data->time3 = (end_close_time - end_open_time) * TIME_COEF;

			data->center_speed1 = center_open_time ? (SENSOR_DISTANCE * F_CPU) / (center_open_time * COUNTER_FREQ_DIV) : 0;
			data->end_speed1 = end_open_time ? (SENSOR_DISTANCE * 2 * F_CPU) / (end_open_time * COUNTER_FREQ_DIV) : 0;

			data->center_speed2 = center_close_time ? (SENSOR_DISTANCE * F_CPU) / ((center_close_time - begin_close_time) * COUNTER_FREQ_DIV) : 0;
			data->end_speed2 = end_close_time ? (SENSOR_DISTANCE * 2 * F_CPU) / ((end_close_time - begin_close_time) * COUNTER_FREQ_DIV) : 0;
		}
	}
}

ISR(PCINT1_vect)
{
	timer_value &= 0xffff0000;
	timer_value |= TCNT1;

	if (SENSOR_CHANGED(0) && IS_OPEN(0)) {
		// Main sensor has just opened.

		// Measuring started.
		measuring = true;

		// Reset main timer.
		timer_value = 0;
		TCNT1 = 0;

		begin_close_time = 0;
		center_open_time = 0;
		center_close_time = 0;
		end_open_time = 0;
		end_close_time = 0;

		status = BEGIN_OPEN;
	}

	if (SENSOR_CHANGED(0) && IS_CLOSED(0) && measuring) {
		// Main sensor has just closed.
		begin_close_time = timer_value;

		status |= BEGIN_CLOSE;
	}

	if (SENSOR_CHANGED(1) && IS_OPEN(1) && measuring) {
		// Center sensor has just opened.
		center_open_time = timer_value;

		status |= CENTER_OPEN;
	}

	if (SENSOR_CHANGED(1) && IS_CLOSED(1) && measuring) {
		// Center sensor has just closed.
		center_close_time = timer_value;

		status |= CENTER_CLOSE;
	}

	if (SENSOR_CHANGED(2) && IS_OPEN(2) && measuring) {
		// End sensor has just opened.
		end_open_time = timer_value;

		status |= END_OPEN;
	}

	if (SENSOR_CHANGED(2) && IS_CLOSED(2) && measuring) {
		// End sensor has just closed.
		end_close_time = timer_value;

		status |= END_CLOSE;

		// Measuring is done.
		measuring = false;
	}

	sensor_state = PINC & 0x07;
}

ISR(TIMER1_OVF_vect)
{
	// Main timer overflow.

	timer_value += 0x10000;
}

void measuring_init(void)
{
	DDRC = 0x00;
	PORTC = 0x00;

	sensor_state = PINC & 0x07;

	// Configure interrupts.
	PCICR |= _BV(PCIE1);
	PCMSK1 = _BV(PCINT8) | _BV(PCINT9) | _BV(PCINT10);

	// Initialize main timer.
	TCCR1A = 0x00;
	//TCCR1B = _BV(CS10) | _BV(CS12); // Set frequency divider to 1024.
	//TCCR1B = _BV(CS12); // Set frequency divider to 256.
	TCCR1B = _BV(CS11); // Set frequency divider to 8.
	TIMSK1 = _BV(TOIE2); // Interrupt on overflow.

	sei();
}

