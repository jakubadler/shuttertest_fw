#ifndef SHUTTER_TEST__DATA_TYPES__H__
#define SHUTTER_TEST__DATA_TYPES__H__

#include <stdbool.h>

struct display_data
{
	float time1;
	float time2;
	float time3;
	float center_open_delay;
	float end_open_delay;
	float center_close_delay;
	float end_close_delay;

	bool measuring;
	//bool error;
};

#endif
