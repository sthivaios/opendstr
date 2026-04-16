#include "../Inc/shutter.h"

#include "main.h"

static volatile uint32_t ticks_when_shutter_began_firing = 0;
static volatile uint32_t shutter_time_to_hold_ms = 0;

void shutter_begin_fire(const uint32_t shutter_time_to_hold) {
  shutter_time_to_hold_ms = shutter_time_to_hold;
  ticks_when_shutter_began_firing = sys_get_ticks();
  HAL_GPIO_WritePin(SHUTTER_OPT_GPIO_Port, SHUTTER_OPT_Pin, 1);
  HAL_GPIO_WritePin(FOCUS_OPT_GPIO_Port, FOCUS_OPT_Pin, 1);
}

void shutter_end_fire(void) {
  if (sys_get_ticks() - ticks_when_shutter_began_firing > shutter_time_to_hold_ms) {
    HAL_GPIO_WritePin(SHUTTER_OPT_GPIO_Port, SHUTTER_OPT_Pin, 0);
    HAL_GPIO_WritePin(FOCUS_OPT_GPIO_Port, FOCUS_OPT_Pin, 0);
  }
}

void shutter_close() {
  HAL_GPIO_WritePin(SHUTTER_OPT_GPIO_Port, SHUTTER_OPT_Pin, 1);
  HAL_GPIO_WritePin(FOCUS_OPT_GPIO_Port, FOCUS_OPT_Pin, 1);
}
void shutter_open() {
  HAL_GPIO_WritePin(SHUTTER_OPT_GPIO_Port, SHUTTER_OPT_Pin, 0);
  HAL_GPIO_WritePin(FOCUS_OPT_GPIO_Port, FOCUS_OPT_Pin, 0);
}