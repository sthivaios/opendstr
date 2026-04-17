#include "../Inc/sys_state_machine.h"

#include "../Inc/ui_state_machine.h"

// main system state
static volatile SystemState_t SystemState = SYS_IDLE;
// TIM1 tick counter - TIM1 ticks every 1ms
static volatile uint32_t ticks = 0;

// interval between shutter fires
static uint32_t user_interval = 5000;
// bulb_mode duration
static volatile uint32_t bulb_mode_duration_ms = 10000;
static volatile uint32_t timestamp_when_bulb_mode_started = 0;
static bool bulb_mode_has_fired = false;
// last request to change the state of the state machine - used for debugging
// the button by checking how much time has passed
static volatile uint32_t last_request_time = 0;
// incremented every time TIM1 ticks, then added by the state machine to the
// ticks variable
static volatile uint32_t tim1_ticks_to_be_added = 0;
// flag that asks the shutter fire handler to actually fire the shutter
static volatile bool shutter_request_flag = false;
// last tick value of tim1 when the shutter was fired
static volatile uint32_t last_fire_time = 0;
// timestamp EXACTLY when the button was pressed
static volatile uint32_t last_button_press_time = 0;
// whether the run/stop button is being held down at the moment
static volatile bool sys_button_is_being_held_down = false;
// shots fired
static volatile int shots_fired = 0;
static volatile int number_of_shots_to_take = 5;
static volatile uint32_t time_remaining_until_shot = 0;
static volatile bool muted = false;

// predefined beep tones:

const BeepEvent_T Sys_State_Machine_Enter_Run_State_Beep = {
    .arr = 13, .psc = 2550, .duty = 50, .duration = 1000};
const BeepEvent_T Sys_State_Machine_Enter_Idle_State_Beep = {
    .arr = 999, .psc = 124, .duty = 50, .duration = 1000};
const BeepEvent_T Sys_State_Machine_Shutter_Fired_Beep = {
    .arr = 999, .psc = 99, .duty = 30, .duration = 150};

// ReSharper disable once CppParameterMayBeConstPtrOrRef <-- this is just a
// JetBrains CLion comment because it whines about this this is the interrupt
// callback for when TIM1 (the main millisecond timer) ticks. it adds its ticks
// to the "tim1_ticks_to_be_added" variable, which, those ticks are then added
// by the state machine to the "ticks" variable
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM1) {
    tim1_ticks_to_be_added++;
  }
}

// returns the current value of the ticks variable - used for timekeeping
uint32_t sys_get_ticks(void) { return ticks; }

// sets the current tick value as the value of the last_fire_time variable
void sys_update_last_fire_time() { last_fire_time = ticks; }

// sets the current tick value as the value of the last_request_time variable
void sys_update_last_state_change_request_time() { last_request_time = ticks; }

// sets the current tick value as the value of the last_button_press_time
// variable
void sys_update_last_button_press_time(void) { last_button_press_time = ticks; }

// sets the value of the "sys_button_is_being_held_down" variable to the
// argument that's passed into this function
void sys_update_button_is_being_held_down(const bool state) {
  if (state != sys_button_is_being_held_down) {
    sys_button_is_being_held_down = state;
  }
}

uint32_t sys_get_user_interval_between_shots(void) { return user_interval; }
void sys_set_user_interval_between_shots(const uint32_t interval) {
  user_interval = interval;
}
int sys_get_number_of_shots_to_take(void) { return number_of_shots_to_take; }
int sys_get_number_of_shots_fired(void) { return shots_fired; }
void sys_set_number_of_shots_to_take(const int number_of_shots) {
  number_of_shots_to_take = number_of_shots;
}
uint32_t sys_get_time_remaining_until_shot(void) {
  return time_remaining_until_shot;
}

bool sys_get_muted(void) { return muted; }
void sys_set_muted(const bool state) { muted = state; }

uint32_t sys_get_bulb_mode_duration(void) {
  return bulb_mode_duration_ms;
}
void sys_set_bulb_mode_duration(const uint32_t duration_ms) {
  bulb_mode_duration_ms = duration_ms;
}

// requests a shutter fire from the state machine by setting the
// "shutter_request_flag" to true
void sys_request_shutter_to_fire(void) { shutter_request_flag = true; }

SystemState_t sys_get_state_machine_state(void) { return SystemState; }

// updates the variables and state of the state machine based on the current
// state of variables and flags
void sys_state_machine_update_state(void) {

  // handle timekeeping with TIM1
  while (tim1_ticks_to_be_added > 0) {
    tim1_ticks_to_be_added--;
    ticks++;
  }

  switch (SystemState) {
  // if the state machine is currently idle
  case SYS_IDLE:
    // if the run/stop button is being held down, and at LEAST the value of
    // "SW_DEBOUNCING_DELAY_MS" has passed since the last button stop
    if (sys_button_is_being_held_down &&
        ((ticks - last_request_time) > SW_DEBOUNCING_DELAY_MS)) {
      // then check if at LEAST the value of "SW_MINIMUM_PRESS_DURATION_MS" has
      // passed since the button was pressed down
      if (ticks - last_button_press_time >= SW_MINIMUM_PRESS_DURATION_MS) {
        // if yes, then:
        // update the last shutter fire timestamp
        sys_update_last_fire_time();
        // update the last state machine change timestamp
        sys_update_last_state_change_request_time();
        // update the last button press timestamp
        sys_update_last_button_press_time();
#if DEBUG
        // debugging line for nucleo boards: the built-in LD2 led turns on to
        // indicate the state machine is in running mode
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);
#endif
        // play tone through the buzzer to indicate that the state has changed
        if (!muted) {
          buzzer_play_tone_for_duration(Sys_State_Machine_Enter_Run_State_Beep,
                                      TIM8);
        }

        // set state machine state to SYS_RUNNING
        SystemState = SYS_RUNNING;
      }
    } else {
      // just update the last shutter fire time
      sys_update_last_fire_time();
    }
    break;

  case SYS_RUNNING:
    // if the user_interval (interval between shutter fires) has passed, then
    // fire the shutter by setting the flag
    // if the run/stop button is being held down, and at LEAST the value of
    // "SW_DEBOUNCING_DELAY_MS" has passed since the last button stop
    if (sys_button_is_being_held_down &&
        ((ticks - last_request_time) > SW_DEBOUNCING_DELAY_MS)) {
      // then check if at LEAST the value of "SW_MINIMUM_PRESS_DURATION_MS" has
      // passed since the button was pressed down
      if (ticks - last_button_press_time >= SW_MINIMUM_PRESS_DURATION_MS) {
#if DEBUG
        // debugging line for nucleo boards: the built-in LD2 led turns off to
        // indicate the state machine is in idle mode
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);
#endif

        // update the last shutter fire timestamp
        sys_update_last_fire_time();
        // update the last state machine change timestamp
        sys_update_last_state_change_request_time();
        // update the last button press timestamp
        sys_update_last_button_press_time();

        // clear the shots_fired variable (reset to 0)
        shots_fired = 0;

        if (ui_state_machine_get_bulb_mode_status()) {
          shutter_open();
        }

        // play tone through the buzzer to indicate that the state has changed
        if (!muted) {
          buzzer_play_tone_for_duration(Sys_State_Machine_Enter_Idle_State_Beep,
                                      TIM8);
        }

        // set state machine state to SYS_IDLE
        SystemState = SYS_IDLE;
      }
        }
    if (!ui_state_machine_get_bulb_mode_status()) {

      if ((ticks - last_fire_time) >= user_interval) {
        sys_update_last_fire_time();
        shutter_request_flag = true;
      }

      if (number_of_shots_to_take - shots_fired <= 0) {
        shutter_request_flag = false;
        shots_fired = 0;
        if (!muted) {
          buzzer_play_tone_for_duration(Sys_State_Machine_Enter_Idle_State_Beep,
                                        TIM8);
        }
        SystemState = SYS_IDLE;
      }
    } else {
      if (!bulb_mode_has_fired) {
        SystemState = SYS_RUNNING;
        shutter_request_flag = true;
      }
    }
    break;
  }
}

void call_shutter_fire_and_clear_flags_and_beep(void) {
  shutter_request_flag = false;
  if (!muted) {
    buzzer_play_tone_for_duration(Sys_State_Machine_Shutter_Fired_Beep, TIM8);
  }
  shutter_begin_fire(TIME_TO_HOLD_SHUTTER_IN_MS);
}

void sys_state_machine_take_action(void) {
  switch (SystemState) {
  case SYS_IDLE:
    if (shutter_request_flag) {
      if (ui_state_machine_get_bulb_mode_status() == false) {
        call_shutter_fire_and_clear_flags_and_beep();
      } else {
        shutter_request_flag = false;
        timestamp_when_bulb_mode_started = ticks;
        shutter_close();
      }
    }
    break;
  case SYS_RUNNING:
    if (!ui_state_machine_get_bulb_mode_status()) {
      if (shutter_request_flag) {
        call_shutter_fire_and_clear_flags_and_beep();
        // increment shots_fired var
        shots_fired++;
      }
    } else {
      if (!bulb_mode_has_fired) {
        shutter_request_flag = false;
        shutter_begin_fire(bulb_mode_duration_ms);
        timestamp_when_bulb_mode_started = ticks;
        bulb_mode_has_fired = true;
      }
    }
    if (!ui_state_machine_get_bulb_mode_status()) {
      uint32_t time_elapsed_since_last_shot = ticks - last_fire_time;
      if (user_interval > time_elapsed_since_last_shot) {
        time_remaining_until_shot = user_interval - time_elapsed_since_last_shot;
      } else {
        time_remaining_until_shot = 0;
      }
    } else {
      time_remaining_until_shot = bulb_mode_duration_ms - (ticks - timestamp_when_bulb_mode_started);
      if (ticks - timestamp_when_bulb_mode_started > bulb_mode_duration_ms) {
        bulb_mode_has_fired = false;
        shutter_open();
        if (!muted) {
          buzzer_play_tone_for_duration(Sys_State_Machine_Enter_Idle_State_Beep,
                                        TIM8);
        }
        SystemState = SYS_IDLE;
      }
    }
    break;
  }
  // used for buzzer timekeeping, if it's time to stop a beep, it will stop the
  // beep
  buzzer_check_and_end_beep(TIM8);
  shutter_end_fire();
}