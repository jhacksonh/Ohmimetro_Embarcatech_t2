#ifndef OLED_LOCAL_H
#define OLED_LOCAL_H

#include <stdlib.h>
#include "pico/stdlib.h"

void oled_Init(uint pin_i2c_sda, uint pin_i2c_scl);
void oled_Draw_draw(uint8_t draw[], uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void oled_Write_Char(char c, uint8_t x, uint8_t y);
void oled_Write_String(const char *str, uint8_t x, uint8_t y);
void oled_Draw_Rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool active, bool fill);
void oled_Bold_Rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void oled_Draw_Line_Vertical(uint8_t x, uint8_t y, uint8_t yEnd, bool active);
void oled_Draw_Line_Horizontal(uint8_t x, uint8_t xEnd, uint8_t y, bool active);
void oled_Update();
void oled_Clear();

#endif
