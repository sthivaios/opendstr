#include "../Inc/shutter_control.h"

static uint32_t tick_count = 0;

// timer callback
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM1) {
    tick_count++;
    if (tick_count >= get_user_interval()) {
      set_fire_flag(true);
      tick_count = 0;
    }
  }
};