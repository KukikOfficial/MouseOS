#ifndef FONT_H
#define FONT_H

#include <stdint.h>

// Массив шрифта 8x8 (базовый набор ASCII)
extern unsigned char font8x8[128][8];

void draw_char(int x, int y, char c, uint32_t color);
void draw_string(int x, int y, const char* str, uint32_t color);

#endif
