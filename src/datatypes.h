#ifndef SHUTTER_TEST__DATA_TYPES__H__
#define SHUTTER_TEST__DATA_TYPES__H__

#include <stdint.h>
#include <stdbool.h>

#define BEGIN_OPEN   (1 << 0)
#define BEGIN_CLOSE  (1 << 1)
#define CENTER_OPEN  (1 << 2)
#define CENTER_CLOSE (1 << 3)
#define END_OPEN     (1 << 4)
#define END_CLOSE    (1 << 5)

struct display_data
{
	int32_t time1; // [us]
	int32_t time2; // [us]
	int32_t time3; // [us]
	int32_t center_speed1; // [um/s]
	int32_t end_speed1; // [um/s]
	int32_t center_speed2; // [um/s]
	int32_t end_speed2; // [um/s]

	uint8_t status;
	bool measuring;
	//bool error;
};

#endif
