#include "../Inc/shutter.h"

#include "main.h"

uint32_t ticks_when_shutter_began_firing = 0;

void shutter_begin_fire(void) {
  ticks_when_shutter_began_firing = sys_get_ticks();
  HAL_GPIO_WritePin(SHUTTER_OPT_GPIO_Port, SHUTTER_OPT_Pin, 1);
  HAL_GPIO_WritePin(FOCUS_OPT_GPIO_Port, FOCUS_OPT_Pin, 1);
}

void shutter_end_fire(int16_t shutter_time_to_hold_ms) {
  if (sys_get_ticks() - ticks_when_shutter_began_firing > shutter_time_to_hold_ms) {
    HAL_GPIO_WritePin(SHUTTER_OPT_GPIO_Port, SHUTTER_OPT_Pin, 0);
    HAL_GPIO_WritePin(FOCUS_OPT_GPIO_Port, FOCUS_OPT_Pin, 0);
  }
}