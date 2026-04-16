#include "../Inc/ui_state_machine.h"

#include "../Inc/buzzer.h"

#include <stdio.h>
#include <string.h>

static volatile UISetting_t UISetting = 1;

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
  buzzer_play_tone_for_duration(Boot_Beep_2, TIM8);
  ssd1306_Fill(Black);
  ssd1306_SetCursor((SSD1306_WIDTH - ((strlen("OpenDSTR")) * 11)) / 2, (SSD1306_HEIGHT - 18) /2 - 12);
  ssd1306_WriteString("OpenDSTR", Font_11x18, White);
  ssd1306_SetCursor((SSD1306_WIDTH - ((strlen("Idle")) * 11)) / 2, ((SSD1306_HEIGHT - 18) /2) + 12);
  ssd1306_WriteString("Idle", Font_11x18, White);
  ssd1306_UpdateScreen();
}

void render_ui(void) {
#define HORIZONTAL_PADDING 8
#define VERTICAL_GAP 4

  ssd1306_Init();
  ssd1306_Fill(Black);
  ssd1306_DrawBitmap(HORIZONTAL_PADDING, 5, epd_bitmap_clock, 18, 18, White);
  ssd1306_SetCursor((SSD1306_WIDTH - (int16_t)(strlen("00:01") * 11) - HORIZONTAL_PADDING),6);
  ssd1306_WriteString("00:01", Font_11x18, White);
  ssd1306_DrawBitmap(HORIZONTAL_PADDING, 24 + VERTICAL_GAP, epd_bitmap_files, 18, 18, White);
  ssd1306_SetCursor((SSD1306_WIDTH - (int16_t)(strlen("67") * 11) - HORIZONTAL_PADDING),(24 + VERTICAL_GAP) + 1);
  ssd1306_WriteString("67", Font_11x18, White);
  ssd1306_SetCursor((SSD1306_WIDTH - (int16_t)(strlen("Status: Idle") * 7) - HORIZONTAL_PADDING) / 2,(SSD1306_HEIGHT - 10));
  ssd1306_WriteString("Status: Idle", Font_7x10, White);
  ssd1306_DrawRectangle(HORIZONTAL_PADDING - 3, (2 + ((UISetting) * 23)), SSD1306_WIDTH - HORIZONTAL_PADDING, (2 + ((UISetting + 1) * 23)), White);
  ssd1306_UpdateScreen();
}