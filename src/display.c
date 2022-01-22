#include "display.h"

#include "stlcd.h"
#include "glcd.h"

#include <stdio.h>

uint8_t buffer[128*64/8];

void display_init(void)
{
	// turn on backlight
	//BLA_DDR |= _BV(BLA);
	//BLA_PORT |= _BV(BLA);

	st7565_init();
	st7565_command(CMD_DISPLAY_ON);
	st7565_command(CMD_SET_ALLPTS_NORMAL);
	st7565_set_brightness(0x07);
}

void display_update(const struct display_data *data)
{
	char buf[64];

	clear_buffer(buffer);

	snprintf(buf, sizeof(buf) - 1, "Time 1: %.5f s", data->time1);
	drawstring(buffer, 0, 0, (uint8_t *) buf);
	snprintf(buf, sizeof(buf) - 1, "Time 2: %.5f s", data->time2);
	drawstring(buffer, 0, 1, (uint8_t *) buf);
	snprintf(buf, sizeof(buf) - 1, "Time 3: %.5f s", data->time3);
	drawstring(buffer, 0, 2, (uint8_t *) buf);

	snprintf(buf, sizeof(buf) - 1, "O. delay 2: %.5f s", data->center_open_delay);
	drawstring(buffer, 0, 3, (uint8_t *) buf);
	snprintf(buf, sizeof(buf) - 1, "O. delay 3: %.5f s", data->end_open_delay);
	drawstring(buffer, 0, 4, (uint8_t *) buf);

	snprintf(buf, sizeof(buf) - 1, "C. delay 2: %.5f s", data->center_close_delay);
	drawstring(buffer, 0, 5, (uint8_t *) buf);
	snprintf(buf, sizeof(buf) - 1, "C. delay 3: %.5f s", data->end_close_delay);
	drawstring(buffer, 0, 6, (uint8_t *) buf);

	if (data->measuring) {
		drawstring(buffer, 0, 7, (uint8_t *) "Measuring...");
	}

	write_buffer(buffer);
}

