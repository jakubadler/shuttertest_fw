#ifndef SHUTTER_TEST__DISPLAY__H__
#define SHUTTER_TEST__DISPLAY__H__

#include "datatypes.h"

void display_init(void);
void display_update(const struct meas_data *data, uint8_t mode);

#endif
