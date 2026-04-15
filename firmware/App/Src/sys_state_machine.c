#include "../Inc/sys_state_machine.h"

#define SW_DEBOUNCING_DELAY_MS 1000
#define SW_MINIMUM_PRESS_DURATION_MS 500

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
// flag that asks the shutter fire handler to actually fire the shutter
static volatile bool shutter_request_flag = false;
// last tick value of tim1 when the shutter was fired
static volatile uint32_t last_fire_time = 0;
// timestamp EXACTLY when the button was pressed
static volatile uint32_t last_button_press_time = 0;
// whether the run/stop button is being held down at the moment
static volatile bool sys_button_is_being_held_down = false;

// ReSharper disable once CppParameterMayBeConstPtrOrRef - clion thing to shut up the warning
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM1) {
    tim1_ticks_to_be_added++;
  }
}

uint32_t sys_get_ticks(void) {
  return ticks;
}

void sys_reset_last_fire_time() {
  last_fire_time = ticks;
}

void sys_reset_last_state_change_request_time() {
  last_request_time = ticks;
}

void sys_update_last_button_press_time(void) {
  last_button_press_time = ticks;
}

void sys_update_button_is_being_held_down(const bool state) {
  if (state != sys_button_is_being_held_down) {
    sys_button_is_being_held_down = state;
  }
}

void sys_state_machine_update_state(void) {

  // handle timing
  while (tim1_ticks_to_be_added > 0) {
    tim1_ticks_to_be_added--;
    ticks++;
  }

  switch (SystemState) {
    case SYS_IDLE:
      if (sys_button_is_being_held_down && ((ticks - last_request_time) > SW_DEBOUNCING_DELAY_MS)) {
        if (ticks - last_button_press_time >= SW_MINIMUM_PRESS_DURATION_MS) {
          sys_reset_last_fire_time();
          sys_reset_last_state_change_request_time();
          sys_update_last_button_press_time();
          HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);
          buzzer_play_tone_for_duration(13, 2550, 50, 1000, TIM8);
          SystemState = SYS_RUNNING;
        }
      } else {
        sys_reset_last_fire_time();
      }
      break;
    case SYS_RUNNING:
      if ((ticks - last_fire_time) >= user_interval) {
        sys_reset_last_fire_time();
        shutter_request_flag = true;
      }
      if (sys_button_is_being_held_down && ((ticks - last_request_time) > SW_DEBOUNCING_DELAY_MS)) {
        if (ticks - last_button_press_time >= SW_MINIMUM_PRESS_DURATION_MS) {
          HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);
          SystemState = SYS_IDLE;
          sys_reset_last_fire_time();
          sys_reset_last_state_change_request_time();
          sys_update_last_button_press_time();
          buzzer_play_tone_for_duration(999, 124, 50, 1000, TIM8);
        }
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
        buzzer_play_tone_for_duration(999, 99, 30, 150, TIM8);
        shutter_fire();
      }
      break;
  }
  buzzer_check_and_end_beep(TIM8);
}