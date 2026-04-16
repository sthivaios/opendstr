#ifndef OPENDSTR_STATE_MACHINE_H
#define OPENDSTR_STATE_MACHINE_H

// includes
#include "../Inc/shutter.h"
#include "buzzer.h"
#include "main.h"
#include "stm32f446xx.h"
#include "stm32f4xx_hal_uart.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// defines
#define SW_DEBOUNCING_DELAY_MS 1000
#define SW_MINIMUM_PRESS_DURATION_MS 500

// typedef
typedef enum {
  SYS_IDLE,
  SYS_RUNNING
} SystemState_t;

// function definitions
void sys_state_machine_update_state(void);
void sys_state_machine_take_action(void);
void sys_update_button_is_being_held_down(bool state);
void sys_update_last_button_press_time(void);
uint32_t sys_get_ticks(void);
void sys_request_shutter_to_fire(void);
SystemState_t sys_get_state_machine_state(void);

#endif //OPENDSTR_STATE_MACHINE_H
