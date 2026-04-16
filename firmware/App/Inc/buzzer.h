#ifndef OPENDSTR_BUZZER_H
#define OPENDSTR_BUZZER_H

#include "sys_state_machine.h"
#include <stdint.h>
#include "stm32f4xx_hal.h"

// beep event: details that the buzzer_play_tone_for_duration() function accepts
// so that it knows what tone to play
typedef struct {
  uint16_t arr;
  uint16_t psc;
  uint16_t duty;
  uint16_t duration;
} BeepEvent_T;

void buzzer_play_tone_for_duration(BeepEvent_T beep_event, TIM_TypeDef *tim);
void buzzer_check_and_end_beep(TIM_TypeDef *tim);

#endif // OPENDSTR_BUZZER_H
