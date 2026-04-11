#include "../Inc/shutter_control.h"

static volatile bool tick_flag = false;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM1) {
    tick_flag = true;
  }
}