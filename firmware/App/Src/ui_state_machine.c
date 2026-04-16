#include "../Inc/ui_state_machine.h"

#include "../Inc/buzzer.h"

#include <stdio.h>
#include <string.h>

#define ENCODER_SW_MINIMUM_PRESS_DURATION_MS 500
#define ENCODER_SW_DEBOUNCING_DELAY_MS 1000

static volatile UISetting_t UISetting = 0;
static volatile UIState_t UIState = 0;

static volatile uint16_t last_encoder_value = 0;

static volatile uint32_t ticks = 0;
static volatile uint32_t timestamp_when_encoder_sw_was_held_down = 0;
static volatile uint32_t timestamp_when_last_encoder_sw_press_was_registered = 0;
static volatile bool encoder_sw_is_being_held_down = false;

void ui_state_machine_toggle_state(void) {
  UIState = UIState == UI_STATE_NAVIGATING ? UI_STATE_EDITING : UI_STATE_NAVIGATING;
}
void ui_state_machine_set_encoder_sw_is_being_held_down(const bool state) {
  encoder_sw_is_being_held_down = state;
}
bool ui_state_machine_get_encoder_sw_is_being_held_down(void) {
  return encoder_sw_is_being_held_down;
}
void ui_state_machine_update_timestamp_when_encoder_sw_was_held_down(void) {
  timestamp_when_encoder_sw_was_held_down = sys_get_ticks();
}
void ui_state_machine_update_timestamp_when_encoder_sw_press_was_registered(void) {
  timestamp_when_last_encoder_sw_press_was_registered = sys_get_ticks();
}

static const unsigned char epd_bitmap_clock [] = {
  // 'clock-4, 18x18px
  0x00, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x0f, 0x3c, 0x00, 0x18, 0x06, 0x00, 0x30, 0xc3, 0x00, 0x20,
  0xc1, 0x00, 0x60, 0xc1, 0x80, 0x40, 0xc0, 0x80, 0x40, 0xc0, 0x80, 0x43, 0xc0, 0x80, 0x47, 0x00,
  0x80, 0x60, 0x01, 0x80, 0x20, 0x03, 0x00, 0x30, 0x03, 0x00, 0x18, 0x06, 0x00, 0x0f, 0x3c, 0x00,
  0x07, 0xf8, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char epd_bitmap_files [] = {
  // 'files, 18x18px
  0x00, 0x00, 0x00, 0x01, 0xf8, 0x00, 0x03, 0x3c, 0x00, 0x03, 0x36, 0x00, 0x03, 0x33, 0x00, 0x33,
  0x1f, 0x00, 0x23, 0x1f, 0x80, 0x63, 0x01, 0x80, 0x63, 0x01, 0x80, 0x63, 0x01, 0x80, 0x63, 0x01,
  0x80, 0x63, 0x01, 0x00, 0x63, 0xff, 0x00, 0x60, 0x7c, 0x00, 0x60, 0x00, 0x00, 0x20, 0x30, 0x00,
  0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00
};

// predefined beeps
const BeepEvent_T Boot_Beep_1 = {.arr = 624, .psc = 249, .duty = 50, .duration = 200};
const BeepEvent_T Boot_Beep_2 = {.arr = 646, .psc = 160, .duty = 50, .duration = 1000};

void convert_milliseconds_to_hr_string(const uint32_t milliseconds, char* buffer) {
  const int32_t total_seconds = milliseconds / 1000;
  const int32_t minutes = total_seconds / 60;
  const int32_t seconds = total_seconds % 60;

  sprintf(buffer, "%02d:%02d", (int)minutes, (int)seconds);
}

void display_shots(const int shutter_fires, const int total_shots) {
  ssd1306_Fill(Black);
  ssd1306_SetCursor((SSD1306_WIDTH - ((strlen("OpenDSTR")) * 11)) / 2, 3);
  ssd1306_WriteString("OpenDSTR", Font_11x18, White);
  ssd1306_SetCursor((SSD1306_WIDTH - (uint16_t)((strlen("SHUTTER HAS FIRED:")) * 6)) / 2, 3 + 18 + 5);
  ssd1306_WriteString("SHUTTER HAS FIRED:", Font_6x8, White);
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "%d/%d", shutter_fires, total_shots);
  ssd1306_SetCursor((SSD1306_WIDTH - ((strlen(buffer)) * 11)) / 2, 3 + 18 + 5 + 15);
  ssd1306_WriteString(buffer, Font_11x18, White);
  ssd1306_UpdateScreen();
}

void display_splash_screen(void) {
  ssd1306_Init();
  // buzzer_play_tone_for_duration(Boot_Beep_2, TIM8);
  ssd1306_Fill(Black);
  ssd1306_SetCursor((SSD1306_WIDTH - ((strlen("OpenDSTR")) * 11)) / 2, (SSD1306_HEIGHT - 18) /2 - 10);
  ssd1306_WriteString("OpenDSTR", Font_11x18, White);
  ssd1306_SetCursor((SSD1306_WIDTH - ((strlen("v0.1.0 Alpha")) * 7)) / 2, ((SSD1306_HEIGHT - 10) /2) + 10);
  ssd1306_WriteString("v0.1.0 Alpha", Font_7x10, White);
  ssd1306_UpdateScreen();
}

void render_ui(void) {
#define HORIZONTAL_PADDING 8
#define VERTICAL_GAP 4

  char buffer[128];

  const SystemState_t sys_state = sys_get_state_machine_state();

  ssd1306_Fill(Black);
  ssd1306_DrawBitmap(HORIZONTAL_PADDING, 5, epd_bitmap_clock, 18, 18, White);
  if (sys_state == SYS_RUNNING) {
    convert_milliseconds_to_hr_string(sys_get_time_remaining_until_shot(), buffer);
    ssd1306_SetCursor((SSD1306_WIDTH - (int16_t)(strlen(buffer) * 11) - HORIZONTAL_PADDING),6);
    ssd1306_WriteString(buffer, Font_11x18, White);
  } else {
    convert_milliseconds_to_hr_string(sys_get_user_interval_between_shots(), buffer);
    ssd1306_SetCursor((SSD1306_WIDTH - (int16_t)(strlen(buffer) * 11) - HORIZONTAL_PADDING),6);
    ssd1306_WriteString(buffer, Font_11x18, White);
  }
  ssd1306_DrawBitmap(HORIZONTAL_PADDING, 24 + VERTICAL_GAP, epd_bitmap_files, 18, 18, White);
  if (sys_state == SYS_RUNNING) {
    sprintf(buffer, "%d/%d", sys_get_number_of_shots_fired(), sys_get_number_of_shots_to_take());
  } else {
    sprintf(buffer, "%d", sys_get_number_of_shots_to_take());
  }
  ssd1306_SetCursor((SSD1306_WIDTH - (int16_t)(strlen(buffer) * 11) - HORIZONTAL_PADDING),(24 + VERTICAL_GAP) + 1);
  ssd1306_WriteString(buffer, Font_11x18, White);
  ssd1306_SetCursor((SSD1306_WIDTH - (int16_t)(strlen(sys_state == SYS_IDLE ? "Status: Idle" : "Status: Running") * 7) - HORIZONTAL_PADDING) / 2,(SSD1306_HEIGHT - 10));
  ssd1306_WriteString((sys_state == SYS_IDLE ? "Status: Idle" : "Status: Running"), Font_7x10, White);
  if (sys_state == SYS_IDLE) {
    ssd1306_DrawRectangle(HORIZONTAL_PADDING - 3, (2 + ((UISetting) * 23)), SSD1306_WIDTH - HORIZONTAL_PADDING, (2 + ((UISetting + 1) * 23)), White);
    if (UIState == UI_STATE_EDITING) {
      ssd1306_InvertRectangle(HORIZONTAL_PADDING - 3, (2 + ((UISetting) * 23)), SSD1306_WIDTH - HORIZONTAL_PADDING, (2 + ((UISetting + 1) * 23)));
    }
  }
  ssd1306_UpdateScreen();
}

void ui_state_machine_update(void) {
  const uint16_t current_encoder_raw = TIM4->CNT;

  const int16_t raw_delta = (int16_t)(current_encoder_raw - last_encoder_value);
  const int16_t delta = raw_delta / 2;

  if (delta != 0) {
    switch (UIState) {
    case UI_STATE_NAVIGATING:
      if (delta > 0) {
        if (UISetting < UI_SETTING_COUNT - 1) {
          UISetting++;
        } else {
          UISetting = 0;
        }
      } else if (delta < 0) {
        if (UISetting == 0) {
          UISetting = UI_SETTING_COUNT - 1;
        } else {
          UISetting--;
        }
      }
      break;

    case UI_STATE_EDITING:
      if (UISetting == UI_SETTING_SHOT_COUNT) {
        sys_set_number_of_shots_to_take(sys_get_number_of_shots_to_take() + delta);
      } else if (UISetting == UI_SETTING_INTERVAL) {
        sys_set_user_interval_between_shots(sys_get_user_interval_between_shots() + (delta * 1000));
      }
      break;
    }

    last_encoder_value += (delta * 2);
  }

  ticks = sys_get_ticks();
  if (ticks - timestamp_when_last_encoder_sw_press_was_registered >= ENCODER_SW_DEBOUNCING_DELAY_MS) {
    if (encoder_sw_is_being_held_down && (ticks - timestamp_when_encoder_sw_was_held_down) >= ENCODER_SW_MINIMUM_PRESS_DURATION_MS) {
      ui_state_machine_toggle_state();
      ui_state_machine_update_timestamp_when_encoder_sw_press_was_registered();
    }
  }

  render_ui();
}