#include "../Inc/buzzer.h"

#include <stdint.h>

static volatile uint32_t ticks_at_start = 0;
static volatile uint16_t beep_duration = 0;

void buzzer_play_tone_for_duration(const uint16_t arr, const uint16_t psc, const uint16_t duty, const uint16_t duration_in_ms, TIM_TypeDef *tim) {
  tim->ARR = arr;
  tim->PSC = psc;
  const int32_t calculated_ccr = (duty * (arr + 1)) / 100;
  tim->CCR1 = calculated_ccr;
  ticks_at_start = sys_get_ticks();
  beep_duration = duration_in_ms;
}

void buzzer_check_and_end_beep(TIM_TypeDef *tim) {
  if ((sys_get_ticks() - ticks_at_start) > beep_duration) {
    tim->CCR1 = 0;
  }
}