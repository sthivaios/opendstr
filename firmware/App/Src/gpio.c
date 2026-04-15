#include "../Inc/gpio.h"

#include "main.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == RUN_STOP_BTN_Pin) {
    if (((GPIOA->IDR >> 8) & 0x1) == 0) {
      sys_update_button_is_being_held_down(true);
      sys_update_last_button_press_time();
    } else {
      sys_update_button_is_being_held_down(false);
    }
  } else {
    __NOP();
  }
}