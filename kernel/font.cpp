#include "include/font.h"
#include "include/vesa.h"

// Возвращает битовую маску строки (8 бит) для конкретного символа
static unsigned char get_font_line(char c, int line) {
    switch (c) {
        case 'M': { unsigned char b[] = {0x44,0x6C,0x54,0x54,0x44,0x44,0x44,0x00}; return b[line]; }
        case 'O': { unsigned char b[] = {0x38,0x44,0x44,0x44,0x44,0x44,0x38,0x00}; return b[line]; }
        case 'U': { unsigned char b[] = {0x44,0x44,0x44,0x44,0x44,0x44,0x38,0x00}; return b[line]; }
        case 'S': { unsigned char b[] = {0x38,0x44,0x30,0x08,0x04,0x44,0x38,0x00}; return b[line]; }
        case 'E': { unsigned char b[] = {0x7C,0x40,0x40,0x78,0x40,0x40,0x7C,0x00}; return b[line]; }
        case 'C': { unsigned char b[] = {0x38,0x44,0x40,0x40,0x40,0x44,0x38,0x00}; return b[line]; }
        case 'R': { unsigned char b[] = {0x78,0x44,0x44,0x78,0x4C,0x46,0x43,0x00}; return b[line]; }
        case ' ': { return 0x00; }
        default:  { unsigned char b[] = {0xFF,0x81,0x81,0x81,0x81,0x81,0xFF,0x00}; return b[line]; } // Квадратик
    }
}

void draw_char(int x, int y, char c, uint32_t color) {
    for (int i = 0; i < 8; i++) {
        unsigned char bits = get_font_line(c, i);
        for (int j = 0; j < 8; j++) {
            if (bits & (0x80 >> j)) {
                put_pixel(x + j, y + i, color);
            }
        }
    }
}

void draw_string(int x, int y, const char* str, uint32_t color) {
    int cur_x = x;
    for (int i = 0; str[i] != '\0'; i++) {
        draw_char(cur_x, y, str[i], color);
        cur_x += 8;
    }
}
