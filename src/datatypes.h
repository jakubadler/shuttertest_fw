#ifndef SHUTTER_TEST__DATA_TYPES__H__
#define SHUTTER_TEST__DATA_TYPES__H__

#include <stdbool.h>

struct display_data
{
	float time1;
	float time2;
	float time3;
	float first_speed1;
	float first_speed2;

	bool measuring;
	bool error;
};

#endif
