#ifndef SHUTTER_TEST__DATA_TYPES__H__
#define SHUTTER_TEST__DATA_TYPES__H__

#include <stdbool.h>

struct display_data
{
	float time1; // [s]
	float time2; // [s]
	float time3; // [s]
	float center_speed1; // [m/s]
	float end_speed1; // [m/s]
	float center_speed2; // [m/s]
	float end_speed2; // [m/s]

	bool measuring;
	//bool error;
};

#endif
