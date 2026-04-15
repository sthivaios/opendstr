#ifndef OPENDSTR_BUZZER_H
#define OPENDSTR_BUZZER_H

#include "sys_state_machine.h"

void buzzer_play_tone_for_duration(const uint16_t arr, const uint16_t psc, const uint16_t duty, const uint16_t duration_in_ms, TIM_TypeDef *tim);
void buzzer_check_and_end_beep(TIM_TypeDef *tim);

#endif //OPENDSTR_BUZZER_H
