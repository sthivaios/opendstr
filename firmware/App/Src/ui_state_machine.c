#include "../Inc/ui_state_machine.h"

#include "../Inc/buzzer.h"

#include <stdio.h>
#include <string.h>

#define UI_BUTTON_MINIMUM_PRESS_DURATION_MS 50
#define UI_BUTTON_DEBOUNCING_DELAY_MS 1000

static volatile UISetting_t UISetting = 0;
static volatile UIState_t UIState = 0;

static volatile uint16_t last_encoder_value = 0;

static volatile uint32_t ticks = 0;
static volatile uint32_t timestamp_when_encoder_sw_was_held_down = 0;
static volatile uint32_t timestamp_when_last_encoder_sw_press_was_registered = 0;
static volatile bool encoder_sw_is_being_held_down = false;

static volatile bool bulb_mode_is_being_held_down = false;
static volatile uint32_t timestamp_when_bulb_mode_was_held_down = 0;
static volatile uint32_t timestamp_when_last_bulb_mode_press_was_registered = 0;

static volatile bool about_page_2 = false;

static bool bulb_mode = false;

bool ui_state_machine_get_bulb_mode_status() {
  return bulb_mode;
}

bool ui_state_machine_get_bulb_mode_is_being_held_down(void) {
  return bulb_mode_is_being_held_down;
}
void ui_state_machine_set_bulb_mode_is_being_held_down(const bool state) {
  bulb_mode_is_being_held_down = state;
}
void ui_state_machine_update_timestamp_when_bulb_mode_press_was_registered(void) {
  timestamp_when_last_bulb_mode_press_was_registered = sys_get_ticks();
}
void ui_state_machine_update_timestamp_when_bulb_mode_was_held_down(void) {
  timestamp_when_bulb_mode_was_held_down = sys_get_ticks();
}

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

const unsigned char epd_bitmap_timer [] = {
  // 'timer, 18x18px
  0x00, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x07,
  0x38, 0x00, 0x0c, 0x0c, 0x00, 0x18, 0x06, 0x00, 0x10, 0x32, 0x00, 0x30, 0x63, 0x00, 0x30, 0xc3,
  0x00, 0x30, 0x03, 0x00, 0x10, 0x02, 0x00, 0x18, 0x06, 0x00, 0x0c, 0x0c, 0x00, 0x06, 0x18, 0x00,
  0x03, 0xf0, 0x00, 0x00, 0x00, 0x00
};

const unsigned char epd_bitmap_info [] = {
  // 'info, 24x24px
  0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x03, 0xff, 0xc0, 0x07, 0xc3, 0xe0, 0x0e, 0x00, 0x70, 0x1c,
  0x00, 0x38, 0x18, 0x00, 0x18, 0x30, 0x18, 0x0c, 0x30, 0x18, 0x0c, 0x60, 0x00, 0x06, 0x60, 0x00,
  0x06, 0x60, 0x18, 0x06, 0x60, 0x18, 0x06, 0x60, 0x18, 0x06, 0x60, 0x18, 0x06, 0x30, 0x18, 0x0c,
  0x38, 0x18, 0x1c, 0x18, 0x00, 0x18, 0x1c, 0x00, 0x38, 0x0e, 0x00, 0x70, 0x07, 0xc3, 0xe0, 0x03,
  0xff, 0xc0, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00
};

// 'volume-2', 18x18px
const unsigned char epd_bitmap_volume_on [] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x03, 0x80, 0x00, 0x07, 0x83, 0x00, 0x3d,
  0x81, 0x00, 0x79, 0x99, 0x80, 0x41, 0x98, 0x80, 0x41, 0x8c, 0x80, 0x41, 0x8c, 0x80, 0x41, 0x98,
  0x80, 0x79, 0x99, 0x80, 0x7d, 0x81, 0x00, 0x07, 0x83, 0x00, 0x03, 0x80, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'volume-off', 18x18px
const unsigned char epd_bitmap_volume_off [] = {
  0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x71, 0x80, 0x00, 0x39, 0x80, 0x00, 0x1c, 0x03, 0x00, 0x3e,
  0x01, 0x00, 0x7f, 0x19, 0x80, 0x43, 0x88, 0x80, 0x41, 0xc8, 0x80, 0x41, 0xe0, 0x80, 0x41, 0xf0,
  0x80, 0x79, 0xb9, 0x80, 0x7d, 0x9c, 0x00, 0x07, 0x8e, 0x00, 0x03, 0x87, 0x00, 0x01, 0x03, 0x80,
  0x00, 0x01, 0x80, 0x00, 0x00, 0x00
};
const unsigned char epd_bitmap_book [] = {
  // 'book, 18x18px
  0x00, 0x00, 0x00, 0x1c, 0x0e, 0x00, 0x7f, 0x3f, 0x80, 0x41, 0xe0, 0x80, 0x40, 0xc0, 0x80, 0x40,
  0xc0, 0x80, 0x40, 0xc0, 0x80, 0x40, 0xc0, 0x80, 0x40, 0xc0, 0x80, 0x40, 0xc0, 0x80, 0x40, 0xc0,
  0x80, 0x40, 0xc0, 0x80, 0x40, 0xc0, 0x80, 0x7f, 0xff, 0x80, 0x01, 0xe0, 0x00, 0x00, 0xc0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const unsigned char epd_bitmap_arrow_down [] = {
  // 'arrow_down, 10x10px
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x12, 0x00, 0x0c, 0x00,
  0x00, 0x00, 0x00, 0x00
};
const unsigned char epd_bitmap_arrow_up [] = {
  // 'arrow-up, 10x10px
  0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x12, 0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};
const unsigned char epd_bitmap_qr_code [] = {
	// 'odsci_qr_code, 62x62px
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x7e, 0x33, 0x03, 0xc1, 0xff, 0xf8,
	0xff, 0xfe, 0x7e, 0x33, 0x03, 0xc1, 0xff, 0xf8, 0xc0, 0x06, 0x7e, 0x00, 0x0c, 0xc1, 0x80, 0x18,
	0xc0, 0x06, 0x7e, 0x00, 0x0c, 0xc1, 0x80, 0x18, 0xc7, 0xe6, 0x67, 0xcc, 0xf0, 0xf9, 0x9f, 0x98,
	0xc7, 0xe6, 0x67, 0xcc, 0xf0, 0xf9, 0x9f, 0x98, 0xc7, 0xe6, 0x79, 0xc3, 0xcc, 0x39, 0x9f, 0x98,
	0xc7, 0xe6, 0x79, 0xc3, 0xcc, 0x39, 0x9f, 0x98, 0xc7, 0xe6, 0x00, 0x0f, 0x03, 0x39, 0x9f, 0x98,
	0xc7, 0xe6, 0x00, 0x0f, 0x03, 0x39, 0x9f, 0x98, 0xc0, 0x06, 0x61, 0xff, 0xcc, 0xc1, 0x80, 0x18,
	0xc0, 0x06, 0x61, 0xff, 0xcc, 0xc1, 0x80, 0x18, 0xff, 0xfe, 0x66, 0x33, 0x33, 0x39, 0xff, 0xf8,
	0xff, 0xfe, 0x66, 0x33, 0x33, 0x39, 0xff, 0xf8, 0x00, 0x00, 0x18, 0x33, 0x00, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x18, 0x33, 0x00, 0xf8, 0x00, 0x00, 0xf8, 0x7e, 0x01, 0xcf, 0x33, 0xc0, 0x67, 0xf8,
	0xf8, 0x7e, 0x01, 0xcf, 0x33, 0xc0, 0x67, 0xf8, 0xc6, 0x01, 0x9e, 0x30, 0xc0, 0x00, 0x79, 0x80,
	0xc6, 0x01, 0x9e, 0x30, 0xc0, 0x00, 0x79, 0x80, 0xc6, 0x01, 0x9e, 0x30, 0xc0, 0x00, 0x79, 0x80,
	0x01, 0xe7, 0x81, 0xff, 0xf3, 0xc1, 0x9f, 0x98, 0x01, 0xe7, 0x81, 0xff, 0xf3, 0xc1, 0x9f, 0x98,
	0x06, 0x78, 0x67, 0xcc, 0x0f, 0xc6, 0x06, 0x00, 0x06, 0x78, 0x67, 0xcc, 0x0f, 0xc6, 0x06, 0x00,
	0xc1, 0x86, 0x06, 0x3c, 0x3c, 0xc1, 0xff, 0xe0, 0xc1, 0x86, 0x06, 0x3c, 0x3c, 0xc1, 0xff, 0xe0,
	0x39, 0x99, 0x98, 0x0f, 0xc3, 0xc0, 0x01, 0x80, 0x39, 0x99, 0x98, 0x0f, 0xc3, 0xc0, 0x01, 0x80,
	0x39, 0xfe, 0x1e, 0x00, 0xc3, 0xc1, 0xff, 0xe0, 0x39, 0xfe, 0x1e, 0x00, 0xc3, 0xc1, 0xff, 0xe0,
	0xff, 0xe1, 0xe6, 0x33, 0x0c, 0xf8, 0x00, 0x60, 0xff, 0xe1, 0xe6, 0x33, 0x0c, 0xf8, 0x00, 0x60,
	0xc0, 0x07, 0x87, 0xcf, 0x03, 0xc6, 0x67, 0xe0, 0xc0, 0x07, 0x87, 0xcf, 0x03, 0xc6, 0x67, 0xe0,
	0xfe, 0x60, 0x18, 0x30, 0x30, 0x06, 0x79, 0x98, 0xfe, 0x60, 0x18, 0x30, 0x30, 0x06, 0x79, 0x98,
	0x06, 0x07, 0xff, 0xff, 0xf3, 0xc1, 0xff, 0xf8, 0x06, 0x07, 0xff, 0xff, 0xf3, 0xc1, 0xff, 0xf8,
	0x06, 0x07, 0xff, 0xff, 0xf3, 0xc1, 0xff, 0xf8, 0x01, 0x98, 0x79, 0xcc, 0x00, 0xfe, 0x00, 0x60,
	0x01, 0x98, 0x79, 0xcc, 0x00, 0xfe, 0x00, 0x60, 0xfe, 0x7e, 0x06, 0x3c, 0x33, 0xff, 0xf8, 0x00,
	0xfe, 0x7e, 0x06, 0x3c, 0x33, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x66, 0x0f, 0xf0, 0x38, 0x1f, 0x80,
	0x00, 0x00, 0x66, 0x0f, 0xf0, 0x38, 0x1f, 0x80, 0xff, 0xfe, 0x00, 0x00, 0x33, 0xf9, 0x9f, 0x98,
	0xff, 0xfe, 0x00, 0x00, 0x33, 0xf9, 0x9f, 0x98, 0xc0, 0x06, 0x66, 0x33, 0x00, 0x38, 0x19, 0xe0,
	0xc0, 0x06, 0x66, 0x33, 0x00, 0x38, 0x19, 0xe0, 0xc7, 0xe6, 0x67, 0xcf, 0x3c, 0xff, 0xfe, 0x18,
	0xc7, 0xe6, 0x67, 0xcf, 0x3c, 0xff, 0xfe, 0x18, 0xc7, 0xe6, 0x06, 0x30, 0xf0, 0xc0, 0x67, 0xe0,
	0xc7, 0xe6, 0x06, 0x30, 0xf0, 0xc0, 0x67, 0xe0, 0xc7, 0xe6, 0x07, 0xff, 0x33, 0xc6, 0x19, 0xf8,
	0xc7, 0xe6, 0x07, 0xff, 0x33, 0xc6, 0x19, 0xf8, 0xc0, 0x06, 0x67, 0xcc, 0x3f, 0x3f, 0xff, 0x80,
	0xc0, 0x06, 0x67, 0xcc, 0x3f, 0x3f, 0xff, 0x80, 0xc0, 0x06, 0x67, 0xcc, 0x3f, 0x3f, 0xff, 0x80,
	0xff, 0xfe, 0x66, 0x3c, 0x3f, 0x01, 0x98, 0x60, 0xff, 0xfe, 0x66, 0x3c, 0x3f, 0x01, 0x98, 0x60
};

// predefined beeps
const BeepEvent_T Boot_Beep_1 = {.arr = 624, .psc = 249, .duty = 50, .duration = 200};
const BeepEvent_T Boot_Beep_2 = {.arr = 646, .psc = 160, .duty = 50, .duration = 1000};

void convert_milliseconds_to_hr_string(const uint32_t milliseconds, char* buffer) {
  const uint32_t total_seconds = milliseconds / 1000;
  const uint32_t minutes = total_seconds / 60;
  const uint32_t seconds = total_seconds % 60;

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

void display_bulb_notice(void) {
  ssd1306_Fill(Black);
  ssd1306_DrawBitmap((SSD1306_WIDTH - 24)/2, 1, epd_bitmap_info, 24, 24, White);
  ssd1306_SetCursor((SSD1306_WIDTH - strlen("In bulb mode,") * 7 )/2, 28);
  ssd1306_WriteString("In bulb mode,", Font_7x10, White);
  ssd1306_SetCursor((SSD1306_WIDTH - strlen("make sure to use") * 7 )/2, 40);
  ssd1306_WriteString("make sure to use", Font_7x10, White);
  ssd1306_SetCursor((SSD1306_WIDTH - strlen("manual focus.") * 7 )/2, 52);
  ssd1306_WriteString("manual focus.", Font_7x10, White);
  ssd1306_UpdateScreen();
}

void display_about(void) {
  if (!about_page_2) {
    ssd1306_Fill(Black);
    ssd1306_DrawBitmap((SSD1306_WIDTH - 18)/2, 1, epd_bitmap_book, 18, 18, White);
    ssd1306_SetCursor((SSD1306_WIDTH - strlen("OpenDSTR Firmware") * 7 )/2, 22);
    ssd1306_WriteString("OpenDSTR Firmware", Font_7x10, White);
    ssd1306_SetCursor((SSD1306_WIDTH - strlen("v0.2.0 Alpha") * 7 )/2, 34);
    ssd1306_WriteString("v0.2.0 Alpha", Font_7x10, White);
    ssd1306_SetCursor((SSD1306_WIDTH - (int16_t)strlen("(C) Stratos Thivaios") * 6 )/2, 46);
    ssd1306_WriteString("(C) Stratos Thivaios", Font_6x8, White);
    ssd1306_SetCursor((SSD1306_WIDTH - (int16_t)strlen("2026") * 6 )/2, 56);
    ssd1306_WriteString("2026", Font_6x8, White);
    ssd1306_DrawBitmap((SSD1306_WIDTH - 11), 54, epd_bitmap_arrow_down, 10, 10, White);
  } else {
    ssd1306_Fill(Black);
    ssd1306_DrawBitmap((SSD1306_WIDTH - 62)/2, 1, epd_bitmap_qr_code, 62, 62, White);
    ssd1306_DrawBitmap((SSD1306_WIDTH - 11), 0, epd_bitmap_arrow_up, 10, 10, White);
  }
  ssd1306_UpdateScreen();
}

void render_ui(void) {
#define HORIZONTAL_PADDING 8
#define VERTICAL_GAP 4

  char buffer[128];

  const SystemState_t sys_state = sys_get_state_machine_state();

  if (UIState == UI_STATE_EDITING && UISetting == UI_SETTING_ABOUT) {
    display_about();
    return;
  }

  if (!bulb_mode) {
    if (UISetting <= 1) {
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
    } else {
      ssd1306_Fill(Black);
      if (!sys_get_muted()) {
        ssd1306_DrawBitmap(HORIZONTAL_PADDING, 5, epd_bitmap_volume_on, 18, 18, White);
        ssd1306_SetCursor((SSD1306_WIDTH - (int16_t)(strlen("Unmuted") * 11) - HORIZONTAL_PADDING),6);
        ssd1306_WriteString("Unmuted", Font_11x18, White);
      } else {
        ssd1306_DrawBitmap(HORIZONTAL_PADDING, 5, epd_bitmap_volume_off, 18, 18, White);
        ssd1306_SetCursor((SSD1306_WIDTH - (int16_t)(strlen("Muted") * 11) - HORIZONTAL_PADDING),6);
        ssd1306_WriteString("Muted", Font_11x18, White);
      }
      ssd1306_DrawBitmap(HORIZONTAL_PADDING, 24 + VERTICAL_GAP, epd_bitmap_book, 18, 18, White);
      ssd1306_SetCursor((SSD1306_WIDTH - (int16_t)(strlen("About") * 11) - HORIZONTAL_PADDING),(24 + VERTICAL_GAP) + 1);
      ssd1306_WriteString("About", Font_11x18, White);
    }
    if (sys_state == SYS_IDLE) {
      ssd1306_DrawRectangle(HORIZONTAL_PADDING - 3, (2 + ((UISetting > 1 ? UISetting - 2 : UISetting) * 23)), SSD1306_WIDTH - HORIZONTAL_PADDING, (2 + ((UISetting > 1 ? UISetting -1 : UISetting + 1) * 23)), White);
      if (UIState == UI_STATE_EDITING) {
        ssd1306_InvertRectangle(HORIZONTAL_PADDING - 3, (2 + ((UISetting > 1 ? UISetting - 2 : UISetting) * 23)), SSD1306_WIDTH - HORIZONTAL_PADDING, (2 + ((UISetting > 1 ? UISetting -1: UISetting + 1) * 23)));
      }
    }
  } else {
    ssd1306_Fill(Black);
    ssd1306_SetCursor((SSD1306_WIDTH - (int16_t)(strlen("BULB MODE") * 7))/ 2,4);
    ssd1306_WriteString("BULB MODE", Font_7x10, White);
    ssd1306_DrawBitmap(HORIZONTAL_PADDING, (SSD1306_HEIGHT - 18) / 2, epd_bitmap_clock, 18, 18, White);
    if (sys_state == SYS_RUNNING) {
      convert_milliseconds_to_hr_string(sys_get_time_remaining_until_shot(), buffer);
      ssd1306_SetCursor((SSD1306_WIDTH - (int16_t)(strlen(buffer) * 11) - HORIZONTAL_PADDING),((SSD1306_HEIGHT - 18) / 2) + 1);
      ssd1306_WriteString(buffer, Font_11x18, White);
    } else {
      convert_milliseconds_to_hr_string(sys_get_bulb_mode_duration(), buffer);
      ssd1306_SetCursor((SSD1306_WIDTH - (int16_t)(strlen(buffer) * 11) - HORIZONTAL_PADDING),((SSD1306_HEIGHT - 18) / 2) + 1);
      ssd1306_WriteString(buffer, Font_11x18, White);
    }
    if (sys_state == SYS_IDLE) {
      ssd1306_DrawRectangle(HORIZONTAL_PADDING - 3, ((SSD1306_HEIGHT - 18) / 2)-2, SSD1306_WIDTH - HORIZONTAL_PADDING, ((SSD1306_HEIGHT - 18) / 2)+18+2, White);
      if (UIState == UI_STATE_EDITING) {
        ssd1306_InvertRectangle(HORIZONTAL_PADDING - 3, ((SSD1306_HEIGHT - 18) / 2)-2, SSD1306_WIDTH - HORIZONTAL_PADDING, ((SSD1306_HEIGHT - 18) / 2)+18+2);
      }
    }
  }
  if (!(UIState == UI_STATE_EDITING && UISetting == UI_SETTING_ABOUT)) {
    ssd1306_SetCursor((SSD1306_WIDTH - (int16_t)(strlen(sys_state == SYS_IDLE ? "Status: Idle" : "Status: Running") * 7)) / 2,(SSD1306_HEIGHT - 10));
    ssd1306_WriteString((sys_state == SYS_IDLE ? "Status: Idle" : "Status: Running"), Font_7x10, White);
  }
  ssd1306_UpdateScreen();
}

void ui_state_machine_update(void) {
  ticks = sys_get_ticks();
  render_ui();
  const uint16_t current_encoder_raw = TIM4->CNT;

  const int16_t raw_delta = (int16_t)(current_encoder_raw - last_encoder_value);
  const int16_t delta = raw_delta / 2;

  if (delta != 0) {
    switch (UIState) {
    case UI_STATE_NAVIGATING:
      if (!bulb_mode) {
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
      }
      break;

    case UI_STATE_EDITING:
      if (!bulb_mode) {
        if (UISetting == UI_SETTING_SHOT_COUNT) {
          const int shots_to_take = sys_get_number_of_shots_to_take() + delta;
          sys_set_number_of_shots_to_take(shots_to_take > 0 ? shots_to_take : 1);
        } else if (UISetting == UI_SETTING_INTERVAL) {
          const int32_t new_interval = (int32_t)sys_get_user_interval_between_shots() + (delta * 1000);
          sys_set_user_interval_between_shots(new_interval >= 1000 ? (uint32_t)new_interval : 1000);
        } else if (UISetting == UI_SETTING_MUTE) {
          sys_set_muted(!sys_get_muted());
        } else if (UISetting == UI_SETTING_ABOUT) {
          about_page_2 = !about_page_2;
        }
      } else {
        const int32_t new_interval = (int32_t)sys_get_bulb_mode_duration() + (delta * 1000);
        sys_set_bulb_mode_duration(new_interval >= 1000 ? (uint32_t)new_interval : 1000);
      }
      break;
    }

    last_encoder_value += (delta * 2);
  }

  if (ticks - timestamp_when_last_encoder_sw_press_was_registered >= UI_BUTTON_DEBOUNCING_DELAY_MS) {
    if (encoder_sw_is_being_held_down && (ticks - timestamp_when_encoder_sw_was_held_down) >= UI_BUTTON_MINIMUM_PRESS_DURATION_MS) {
      ui_state_machine_toggle_state();
      ui_state_machine_update_timestamp_when_encoder_sw_press_was_registered();
    }
  }
  if (ticks - timestamp_when_last_bulb_mode_press_was_registered >= UI_BUTTON_DEBOUNCING_DELAY_MS) {
    if (bulb_mode_is_being_held_down && (ticks - timestamp_when_bulb_mode_was_held_down) >= UI_BUTTON_MINIMUM_PRESS_DURATION_MS) {
      bulb_mode = !bulb_mode;
      if (bulb_mode) {
        display_bulb_notice();
        HAL_Delay(3000);
      }
      ui_state_machine_update_timestamp_when_bulb_mode_press_was_registered();
    }
  }
}