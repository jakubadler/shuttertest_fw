#include "measuring.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>

#include <stdbool.h>

#define IS_OPEN(x) (!(PINC & _BV((x))))
#define IS_CLOSED(x) (!(IS_OPEN((x))))
#define SENSOR_CHANGED(x) ((sensor_state & _BV(x)) ^ (PINC & _BV(x)))
#define IS_ACTIVE(x) (counter_active & _BV(x))
#define SET_ACTIVE(x) (counter_active |= _BV(x))
#define SET_INACTIVE(x) (counter_active &= ~_BV(x))

#define SENSOR_DISTANCE 16 // [mm]
#define COUNTER_FREQ_DIV 1024

enum counter
{
	MAIN,
	CENTER_OPEN,
	CENTER_CLOSE,
	END_OPEN,
	END_CLOSE
};

static volatile uint8_t sensor_state = 0;
static volatile uint8_t counter_active = 0;

static volatile bool measuring = false;

static volatile int32_t main_timer_value = 0;
static volatile int32_t center_timer_open_value = 0;
static volatile int32_t center_timer_close_value = 0;
static volatile int32_t end_timer_open_value = 0;
static volatile int32_t end_timer_close_value = 0;

#define TIME_COEF (COUNTER_FREQ_DIV / (F_CPU / 1000000))

void get_measurements(struct display_data *data)
{
	int32_t begin_time, center_time, end_time;
	bool update = false;

	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		data->measuring = measuring;

		if (!measuring) { // Only update data when measuring is done.
			update = true;

			begin_time = main_timer_value;
			center_time = begin_time - center_timer_open_value + center_timer_close_value;
			end_time = begin_time - end_timer_open_value + end_timer_close_value;
		}
	}

	if (update) {

		data->time1 = begin_time * TIME_COEF;
		data->time2 = center_time * TIME_COEF;
		data->time3 = end_time * TIME_COEF;

		data->center_speed1 = center_timer_open_value ? (SENSOR_DISTANCE * F_CPU) / (center_timer_open_value * COUNTER_FREQ_DIV) : 0;
		data->end_speed1 = end_timer_open_value ? (SENSOR_DISTANCE * F_CPU) / (end_timer_open_value * COUNTER_FREQ_DIV) : 0;

		data->center_speed2 = center_timer_close_value ? (SENSOR_DISTANCE * F_CPU) / (center_timer_close_value * COUNTER_FREQ_DIV) : 0;
		data->end_speed2 = end_timer_close_value ? (SENSOR_DISTANCE * F_CPU) / (end_timer_close_value * COUNTER_FREQ_DIV) : 0;
	}
}

ISR(PCINT1_vect)
{
	if (SENSOR_CHANGED(0) && IS_OPEN(0)) {
		// Main sensor has just opened.
		SET_ACTIVE(MAIN);

		// Measuring started.
		measuring = true;

		// Reset main timer.
		main_timer_value = 0;
		TCNT1 = 0;

		// Reset center timer.
		center_timer_open_value = 0;
		TCNT0 = 0;
		SET_ACTIVE(CENTER_OPEN);

		// Reset end timer.
		end_timer_open_value = 0;
		TCNT2 = 0;
		SET_ACTIVE(END_OPEN);
	}

	if (SENSOR_CHANGED(0) && IS_CLOSED(0)) {
		// Main sensor has just closed.
		SET_INACTIVE(MAIN);

		main_timer_value += TCNT1;

		// Reset center timer.
		center_timer_close_value = 0;
		TCNT0 = 0;
		SET_ACTIVE(CENTER_CLOSE);

		// Reset end timer.
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
		SET_INACTIVE(CENTER_OPEN); // Just in case.
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
		SET_INACTIVE(END_OPEN); // Just in case.

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
	PORTC = 0x00;

	sensor_state = PINC & 0x07;

	// Configure interrupts.
	PCICR |= _BV(PCIE1);
	PCMSK1 = _BV(PCINT8) | _BV(PCINT9) | _BV(PCINT10);

	// Initialize main timer.
	TCCR1A = 0x00;
	TCCR1B = _BV(CS10) | _BV(CS12); // Set frequency divider to 1024.
	TIMSK1 = _BV(TOIE2); // Interrupt on overflow.

	// Initialize center timer.
	TCCR0A = 0x00;
	TCCR0B = _BV(CS00) | _BV(CS02); // Set frequency divider to 1024.
	TIMSK0 = _BV(TOIE2); // Interrupt on overflow.

	// Initialize end timer.
	TCCR2A = 0x00;
	TCCR2B = _BV(CS20) | _BV(CS21) | _BV(CS22); // Set frequency divider to 1024. Yes, it's different from counters 0 and 1.
	TIMSK2 = _BV(TOIE2); // Interrupt on overflow.

	sei();
}

