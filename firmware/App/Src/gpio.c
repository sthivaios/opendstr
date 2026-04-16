#include "../Inc/gpio.h"

#include "../Inc/ui_state_machine.h"
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
  } else if (GPIO_Pin == ENCODER_SW_Pin) {
    if (((GPIOB->IDR >> 5) & 0x1) == 0) {
      if (!ui_state_machine_get_encoder_sw_is_being_held_down()) {
        ui_state_machine_set_encoder_sw_is_being_held_down(true);
       ui_state_machine_update_timestamp_when_encoder_sw_was_held_down();
      }
    } else {
      ui_state_machine_set_encoder_sw_is_being_held_down(false);
    }
  } else if (GPIO_Pin == BULB_MODE_BTN_Pin) {
    if (((GPIOA->IDR >> 9) & 0x1) == 0) {
      if (!ui_state_machine_get_bulb_mode_is_being_held_down()) {
        ui_state_machine_update_timestamp_when_bulb_mode_was_held_down();
        ui_state_machine_set_bulb_mode_is_being_held_down(true);
      }
    } else {
      ui_state_machine_set_bulb_mode_is_being_held_down(false);
    }
  }
}