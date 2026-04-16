#ifndef OPENDSTR_UI_STATE_MACHINE_H
#define OPENDSTR_UI_STATE_MACHINE_H

void display_shots(int shutter_fires, int total_shots);
void display_splash_screen(void);
void test_display(void);

typedef enum {
  UI_SETTING_INTERVAL = 0,
  UI_SETTING_SHOT_COUNT,
  UI_SETTING_COUNT  // handy for wrapping the selection
} UISetting_t;

typedef enum {
  UI_STATE_NAVIGATING,  // moving the box between settings
  UI_STATE_EDITING,     // editing a setting value
} UIState_t;

#include "ssd1306.h"
#include "ssd1306_fonts.h"

#endif //OPENDSTR_UI_STATE_MACHINE_H
