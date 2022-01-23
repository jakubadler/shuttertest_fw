#ifndef SHUTTER_TEST__DATA_TYPES__H__
#define SHUTTER_TEST__DATA_TYPES__H__

#include <stdint.h>
#include <stdbool.h>

enum mode
{
	MODE_HORIZ = 0,
	MODE_VERT,
	MODE_SINGLE,
	MODE_LAST
};

struct meas_data
{
	int32_t time1; // [us]
	int32_t time2; // [us]
	int32_t time3; // [us]
	int32_t center_speed1; // [um/s]
	int32_t end_speed1; // [um/s]
	int32_t center_speed2; // [um/s]
	int32_t end_speed2; // [um/s]

	bool measuring;
};

#endif
