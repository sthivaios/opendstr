#ifndef OPENDSTR_STATE_MACHINE_H
#define OPENDSTR_STATE_MACHINE_H

// includes
#include <stdbool.h>
#include <stdint.h>
#include "../Inc/shutter.h"
#include "main.h"
#include "stm32f446xx.h"
#include "stm32f4xx_hal_uart.h"
#include <stdio.h>

// typedef
typedef enum {
  CURSOR_INTERVAL = 0,
  CURSOR_NUM_OF_SHOTS = 1,
  CURSOR_INITIAL_DELAY = 2,
  CURSOR_POSITIONS_COUNT = 3
} CursorState_T;

typedef enum {
  SYS_IDLE,
  SYS_RUNNING
} SystemState_t;

// function definitions
void sys_state_machine_update_state(void);
void sys_state_machine_take_action(void);
void sys_sys_machine_change_state();

#endif //OPENDSTR_STATE_MACHINE_H
