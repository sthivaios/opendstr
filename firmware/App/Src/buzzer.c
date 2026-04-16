#include "../Inc/buzzer.h"

#include <stdint.h>

// flags for keeping track of time
static volatile uint32_t ticks_at_start = 0;
static volatile uint16_t beep_duration = 0;

// begins playing a tone on the buzzer with the specific duration
// accepts a BeepEvent_T type struct
void buzzer_play_tone_for_duration(const BeepEvent_T beep_event,
                                   TIM_TypeDef *tim) {
  tim->ARR = beep_event.arr;
  tim->PSC = beep_event.psc;
  const int32_t calculated_ccr = (beep_event.duty * (beep_event.arr + 1)) / 100;
  tim->CCR1 = calculated_ccr;
  ticks_at_start = sys_get_ticks();
  beep_duration = beep_event.duration;
}

// checks if enough time has passed, and stops the beeping
void buzzer_check_and_end_beep(TIM_TypeDef *tim) {
  if ((sys_get_ticks() - ticks_at_start) > beep_duration) {
    tim->CCR1 = 0;
  }
}