#ifndef OPENDSTR_UI_STATE_MACHINE_H
#define OPENDSTR_UI_STATE_MACHINE_H
#include <stdbool.h>

void display_shots(int shutter_fires, int total_shots);
void display_splash_screen(void);
void render_ui(void);

void ui_state_machine_update(void);
void ui_state_machine_take_action(void);
void ui_state_machine_toggle_state(void);
void ui_state_machine_update_timestamp_when_encoder_sw_was_held_down(void);
void ui_state_machine_set_encoder_sw_is_being_held_down(bool state);
void ui_state_machine_update_timestamp_when_encoder_sw_press_was_registered(void);
bool ui_state_machine_get_encoder_sw_is_being_held_down(void);
void ui_state_machine_set_bulb_mode_is_being_held_down(bool state);
bool ui_state_machine_get_bulb_mode_is_being_held_down(void);
void ui_state_machine_update_timestamp_when_bulb_mode_press_was_registered(void);
void ui_state_machine_update_timestamp_when_bulb_mode_was_held_down(void);
bool ui_state_machine_get_bulb_mode_status(void);

typedef enum {
  UI_SETTING_INTERVAL = 0,
  UI_SETTING_SHOT_COUNT,
  UI_SETTING_MUTE,
  UI_SETTING_ABOUT,
  UI_SETTING_COUNT  // handy for wrapping the selection
} UISetting_t;

typedef enum {
  UI_STATE_NAVIGATING,  // moving the box between settings
  UI_STATE_EDITING,     // editing a setting value
} UIState_t;

#include "ssd1306.h"
#include "ssd1306_fonts.h"

#endif //OPENDSTR_UI_STATE_MACHINE_H
