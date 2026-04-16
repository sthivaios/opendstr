#include "../Inc/ui_state_machine.h"

#include <stdio.h>
#include <string.h>

void display_shots(int shutter_fires, int total_shots) {
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

void splash_display(void) {
  ssd1306_Init();
  ssd1306_Fill(Black);
  ssd1306_SetCursor((SSD1306_WIDTH - ((strlen("OpenDSTR")) * 11)) / 2, (SSD1306_HEIGHT - 18) /2 - 12);
  ssd1306_WriteString("OpenDSTR", Font_11x18, White);
  ssd1306_SetCursor((SSD1306_WIDTH - ((strlen("IDLE")) * 11)) / 2, ((SSD1306_HEIGHT - 18) /2) + 12);
  ssd1306_WriteString("IDLE", Font_11x18, White);
  ssd1306_UpdateScreen();
}