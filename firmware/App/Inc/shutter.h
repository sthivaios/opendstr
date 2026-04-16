#ifndef OPENDSTR_SHUTTER_H
#define OPENDSTR_SHUTTER_H
#include <stdint.h>
#include "stm32f4xx_hal.h"

// function definitions
void shutter_begin_fire(void);
void shutter_end_fire(int16_t shutter_time_to_hold_ms);

#endif //OPENDSTR_SHUTTER_H
