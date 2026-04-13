#include "../Inc/sys_state_machine.h"

#define SW_DEBOUNCING_DELAY_MS 500

// main system state
static volatile SystemState_t SystemState = SYS_IDLE;
// TIM1 tick counter - TIM1 ticks every 1ms
static volatile uint32_t ticks = 0;

// interval between shutter fires
static uint32_t user_interval = 1000;
// last request to change the state of the state machine - used for debugging the button by checking how much time has passed
static volatile uint32_t last_request_time = 0;
// incremented every time TIM1 ticks, then added by the state machine to the ticks variable
static volatile uint32_t tim1_ticks_to_be_added = 0;
// flag to indicate a status change has been requested (the button was pressed)
static volatile bool sys_change_state_requested = false;
// flag that asks the shutter fire handler to actually fire the shutter
static volatile bool shutter_request_flag = false;
// last tick value of tim1 when the shutter was fired
static volatile uint32_t last_fire_time = 0;

// ReSharper disable once CppParameterMayBeConstPtrOrRef - clion thing to shut up the warning
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM1) {
    tim1_ticks_to_be_added++;
  }
}

void sys_sys_machine_change_state() {
  sys_change_state_requested = true;
}

void sys_reset_last_fire_time() {
  last_fire_time = ticks;
}

void sys_state_machine_update_state(void) {

  // handle timing
  while (tim1_ticks_to_be_added > 0) {
    tim1_ticks_to_be_added--;
    ticks++;
  }

  switch (SystemState) {
    case SYS_IDLE:
      if (sys_change_state_requested && ((ticks - last_request_time) > SW_DEBOUNCING_DELAY_MS)) {
        sys_change_state_requested = false;
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);
        SystemState = SYS_RUNNING;
        sys_reset_last_fire_time();
      } else {
        sys_change_state_requested = false;
        sys_reset_last_fire_time();
      }
      break;
    case SYS_RUNNING:
      if ((ticks - last_fire_time) >= user_interval) {
        sys_reset_last_fire_time();
        shutter_request_flag = true;
      }
      if (sys_change_state_requested && ((ticks - last_request_time) > SW_DEBOUNCING_DELAY_MS)) {
        sys_change_state_requested = false;
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);
        SystemState = SYS_IDLE;
        sys_reset_last_fire_time();
      } else {
        sys_change_state_requested = false;
      }
      break;
  }
}

void sys_state_machine_take_action(void) {
  switch (SystemState) {
    case SYS_IDLE:
      break;
    case SYS_RUNNING:
      if (shutter_request_flag) {
        shutter_request_flag = false;
        shutter_fire();
      }
      break;
  }
}