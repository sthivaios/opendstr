#include "../Inc/gpio.h"

#include "main.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == ENCODER_SW_Pin) {
    sys_sys_machine_change_state();
  } else {
    __NOP();
  }
}