#ifndef SSD1306_CONF_H
#define SSD1306_CONF_H

// display size
#define SSD1306_WIDTH  128
#define SSD1306_HEIGHT 64

// the i2c handle
#define SSD1306_I2C_PORT hi2c1

// fonts
#define SSD1306_INCLUDE_FONT_11x18
#define SSD1306_INCLUDE_FONT_6x8

// include main hal header
#include "stm32f4xx_hal.h"

// configure display to use i2c instead of spi
#define SSD1306_USE_I2C

#endif