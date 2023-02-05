#ifndef SHUTTER_TEST__MEASURING__H__
#define SHUTTER_TEST__MEASURING__H__

#include "datatypes.h"

#include <stdbool.h>

void measuring_init(void);
void measuring_init_mode(uint8_t mode);
void get_measurements(struct meas_data *data);

#endif

