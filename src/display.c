#include "display.h"

#include "stlcd.h"
#include "glcd.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint8_t buffer[128*64/8];

void display_init(void)
{
	// turn on backlight
	//BLA_DDR |= _BV(BLA);
	//BLA_PORT |= _BV(BLA);

	st7565_init(true);
	st7565_command(CMD_DISPLAY_ON);
	st7565_command(CMD_SET_ALLPTS_NORMAL);
	st7565_set_brightness(0x07);
}

static inline void drawarrowrl(uint8_t *buf, uint8_t x, uint8_t y, uint8_t len, uint8_t color)
{
	drawline(buf, x - len, y, x + len, y, color);
	drawline(buf, x - len, y, x - len + 3, y + 3, color);
	drawline(buf, x - len, y, x - len + 3, y - 3, color);
}

static inline void drawarrowtd(uint8_t *buf, uint8_t x, uint8_t y, uint8_t len, uint8_t color)
{
	drawline(buf, x, y + len, x, y - len, color);
	drawline(buf, x, y + len, x - 3, y + len - 3, color);
	drawline(buf, x, y + len, x + 3, y + len - 3, color);
}

#define PRECISION 4

void print_number(char *buf, int32_t num)
{
	int32_t d, r;
	uint8_t i = 0, j = 0, k = 0, shift = 0;
	char digits[PRECISION];

	if (num < 0) {
		num = -num;
		buf[0] = '-';
		k++;
	}

	while (1) {
		d = num / 10;
		r = num % 10;

		digits[i % PRECISION] = '0' + r;

		i++;
		num = d;

		if (num == 0 && i != 0) break;
	}

	for (j = 0; j < PRECISION && i > 0; j++) {
		if (j > 0 && i % 3 == 0) {
			shift = i;
			buf[j+k] = '.';
			k++;
		}
		i--;
		buf[j+k] = digits[i % PRECISION];
	}

	j += k;

	buf[j++] = ' ';

	switch (shift) {
	case 0:
		buf[j++] = 'u';
		break;
	case 3:
		buf[j++] = 'm';
		break;
	default:
		break;
	}

	buf[j] = '\0';
}

void display_update(const struct meas_data *data, uint8_t mode)
{
	char textbuf[24];
	char numbuf[PRECISION + 4];

	clear_buffer(buffer);

	print_number(numbuf, data->time1);
	snprintf(textbuf, sizeof(textbuf) - 1, "Time 1: %ss", numbuf);
	drawstring(buffer, 0, 0, (uint8_t *) textbuf);

	if (mode != MODE_SINGLE) {
		print_number(numbuf, data->time2);
		snprintf(textbuf, sizeof(textbuf) - 1, "Time 2: %ss", numbuf);
		drawstring(buffer, 0, 1, (uint8_t *) textbuf);
		print_number(numbuf, data->time3);
		snprintf(textbuf, sizeof(textbuf) - 1, "Time 3: %ss", numbuf);
		drawstring(buffer, 0, 2, (uint8_t *) textbuf);

		print_number(numbuf, data->center_speed1);
		snprintf(textbuf, sizeof(textbuf) - 1, "1st sp1: %sm/s", numbuf);
		drawstring(buffer, 0, 3, (uint8_t *) textbuf);

		print_number(numbuf, data->end_speed1);
		snprintf(textbuf, sizeof(textbuf) - 1, "1st sp2: %sm/s", numbuf);
		drawstring(buffer, 0, 4, (uint8_t *) textbuf);

		print_number(numbuf, data->center_speed2);
		snprintf(textbuf, sizeof(textbuf) - 1, "2nd sp1: %sm/s", numbuf);
		drawstring(buffer, 0, 5, (uint8_t *) textbuf);

		print_number(numbuf, data->end_speed2);
		snprintf(textbuf, sizeof(textbuf) - 1, "2nd sp2: %sm/s", numbuf);
		drawstring(buffer, 0, 6, (uint8_t *) textbuf);
	}

	switch (mode)
	{
	case MODE_HORIZ:
		drawarrowrl(buffer, 120, 60, 4, 1);
		break;
	case MODE_VERT:
		drawarrowtd(buffer, 120, 59, 4, 1);
		break;
	case MODE_SINGLE:
		drawcircle(buffer, 120, 60, 2, 1);
		break;
	default:
		break;
	}

	if (data->measuring) {
		drawstring(buffer, 0, 7, (uint8_t *) "Measuring...");
	} else if ((data->status & 0x3f) != 0x3f) {
		snprintf(textbuf, sizeof(textbuf) - 1, "INVALID: 0x%x", data->status);
		drawstring(buffer, 0, 7, (uint8_t *) textbuf);
	}


	write_buffer(buffer);
}

