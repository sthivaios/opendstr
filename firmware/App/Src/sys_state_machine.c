#include "../Inc/sys_state_machine.h"

// main system state
static volatile SystemState_t SystemState = SYS_IDLE;

static uint32_t user_interval = 10;
static volatile bool sys_tick_flag = false;
static volatile bool trigger_event_flag = false;
static volatile bool shutter_request_flag = false;
static volatile uint32_t tick_count = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM1) {
    sys_tick_flag = true;
  }
}

void sys_sys_machine_change_state() {
  SystemState = (SystemState == SYS_RUNNING) ? SYS_IDLE : SYS_RUNNING;
  tick_count = 0;
}

void sys_state_machine_update_state(void) {

  // handle timing
  if (sys_tick_flag) {
    sys_tick_flag = false;

    tick_count++;

    if (tick_count >= user_interval) {
      trigger_event_flag = true;
      tick_count = 0;
    }
  }

  switch (SystemState) {
    case SYS_IDLE:
      break;
    case SYS_RUNNING:
      if (trigger_event_flag) {
        trigger_event_flag = false;
        shutter_request_flag = true;
      }
      break;
  }
}

void sys_state_machine_take_action(void) {
  switch (SystemState) {
    case SYS_IDLE:
      return;
    case SYS_RUNNING:
      if (shutter_request_flag) {
        shutter_request_flag = false;
        shutter_fire();
      }
      break;
  }
}