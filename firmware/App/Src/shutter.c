#include "../Inc/shutter.h"

#include "main.h"
#include "stm32f4xx_hal_gpio.h"

void shutter_fire(void) {
  HAL_GPIO_WritePin(SHUTTER_OPT_GPIO_Port, SHUTTER_OPT_Pin, 1);
  HAL_GPIO_WritePin(FOCUS_OPT_GPIO_Port, FOCUS_OPT_Pin, 1);
  HAL_Delay(200);
  HAL_GPIO_WritePin(SHUTTER_OPT_GPIO_Port, SHUTTER_OPT_Pin, 0);
  HAL_GPIO_WritePin(FOCUS_OPT_GPIO_Port, FOCUS_OPT_Pin, 0);
}