#include "measuring.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>

#include <stdbool.h>

#include "io.h"

#define IS_CLOSED(x) (!(SENSOR_PIN & _BV((x))))
#define IS_OPEN(x) (!(IS_CLOSED((x))))

#define SENSOR_CHANGED(x) ((sensor_state & _BV(x)) ^ (SENSOR_PIN & _BV(x)))

#define SENSOR_DISTANCE 16 // [mm]
//#define COUNTER_FREQ_DIV 1024
//#define COUNTER_FREQ_DIV 256
#define COUNTER_FREQ_DIV 8

static volatile uint8_t sensor_begin = 0;
static volatile uint8_t sensor_center = 0;
static volatile uint8_t sensor_end = 0;

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

void get_measurements(struct meas_data *data)
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

ISR(PCINT0_vect)
{
	timer_value &= 0xffff0000;
	timer_value |= TCNT1;

	if (SENSOR_CHANGED(sensor_begin) && IS_OPEN(sensor_begin)) {
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

	if (SENSOR_CHANGED(sensor_begin) && IS_CLOSED(sensor_begin) && measuring) {
		// Main sensor has just closed.
		begin_close_time = timer_value;

		status |= BEGIN_CLOSE;
	}

	if (SENSOR_CHANGED(sensor_center) && IS_OPEN(sensor_center) && measuring) {
		// Center sensor has just opened.
		center_open_time = timer_value;

		status |= CENTER_OPEN;
	}

	if (SENSOR_CHANGED(sensor_center) && IS_CLOSED(sensor_center) && measuring) {
		// Center sensor has just closed.
		center_close_time = timer_value;

		status |= CENTER_CLOSE;
	}

	if (SENSOR_CHANGED(sensor_end) && IS_OPEN(sensor_end) && measuring) {
		// End sensor has just opened.
		end_open_time = timer_value;

		status |= END_OPEN;
	}

	if (SENSOR_CHANGED(sensor_end) && IS_CLOSED(sensor_end) && measuring) {
		// End sensor has just closed.
		end_close_time = timer_value;

		status |= END_CLOSE;

		// Measuring is done.
		measuring = false;
	}

	sensor_state = SENSOR_PIN & 0x3f;
}

ISR(TIMER1_OVF_vect)
{
	// Main timer overflow.

	timer_value += 0x10000;
}

void measuring_init(void)
{
	SENSOR_DDR = 0x00;
	SENSOR_PORT = 0x00; // Disable internal pull-up resistor.

	sensor_state = SENSOR_PIN & 0x3f;

	// Configure interrupts.
	PCICR |= _BV(PCIE0);
	PCMSK0 = _BV(PCINT0) | _BV(PCINT1) | _BV(PCINT2) | _BV(PCINT3) | _BV(PCINT4) | _BV(PCINT5);

	// Initialize main timer.
	TCCR1A = 0x00;
	//TCCR1B = _BV(CS10) | _BV(CS12); // Set frequency divider to 1024.
	//TCCR1B = _BV(CS12); // Set frequency divider to 256.
	TCCR1B = _BV(CS11); // Set frequency divider to 8.
	TIMSK1 = _BV(TOIE2); // Interrupt on overflow.

	sei();
}

void measuring_init_mode(uint8_t mode)
{
	switch (mode) {
	default:
	case MODE_HORIZ:
		sensor_begin = 0;
		sensor_center = 1;
		sensor_end = 2;
		break;
	case MODE_VERT:
		sensor_begin = 3;
		sensor_center = 1;
		sensor_end = 4;
		break;
	case MODE_SINGLE:
		sensor_begin = 5;
		sensor_center = 1;
		sensor_end = 5;
		break;
	}
}

