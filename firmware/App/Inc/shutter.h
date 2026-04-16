#ifndef OPENDSTR_SHUTTER_H
#define OPENDSTR_SHUTTER_H
#include <stdint.h>
#include "stm32f4xx_hal.h"

// function definitions
void shutter_begin_fire(uint32_t shutter_time_to_hold);
void shutter_end_fire(void);
void shutter_close(void);
void shutter_open(void);

#endif //OPENDSTR_SHUTTER_H
