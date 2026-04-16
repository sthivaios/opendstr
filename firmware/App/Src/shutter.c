#include "../Inc/shutter.h"

#include "main.h"
#include "stm32f4xx_hal_gpio.h"

#define SHUTTER_TIME_TO_HOLD 400

uint32_t ticks_when_shutter_began_firing = 0;

void shutter_begin_fire(void) {
  ticks_when_shutter_began_firing = sys_get_ticks();
  HAL_GPIO_WritePin(SHUTTER_OPT_GPIO_Port, SHUTTER_OPT_Pin, 1);
  HAL_GPIO_WritePin(FOCUS_OPT_GPIO_Port, FOCUS_OPT_Pin, 1);
}

void shutter_end_fire(void) {
  if (sys_get_ticks() - ticks_when_shutter_began_firing > SHUTTER_TIME_TO_HOLD) {
    HAL_GPIO_WritePin(SHUTTER_OPT_GPIO_Port, SHUTTER_OPT_Pin, 0);
    HAL_GPIO_WritePin(FOCUS_OPT_GPIO_Port, FOCUS_OPT_Pin, 0);
  }
}