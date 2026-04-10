#ifndef OPENDSTR_STATE_MACHINE_H
#define OPENDSTR_STATE_MACHINE_H

// includes
#include <stdbool.h>
#include <stdint.h>
#include "../Inc/shutter.h"

// function definitions
void set_fire_flag(bool flag);
void set_user_interval(uint32_t interval);
uint32_t get_user_interval();

void state_machine_update(void);

#endif //OPENDSTR_STATE_MACHINE_H
