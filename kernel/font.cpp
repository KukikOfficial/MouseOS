#include "include/font.h"
#include "include/vesa.h"

// Упрощенный пример шрифта (буква 'A')
unsigned char font8x8[128][8] = {
    ['A'] = {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00},
    ['B'] = {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
    // ... для полноценной ОС нужно будет заполнить весь массив или загрузить из файла
};

void draw_char(int x, int y, char c, uint32_t color) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (font8x8[(int)c][i] & (1 << (7 - j))) {
                put_pixel(x + j, y + i, color);
            }
        }
    }
}

void draw_string(int x, int y, const char* str, uint32_t color) {
    int cur_x = x;
    for (int i = 0; str[i] != '\0'; i++) {
        draw_char(cur_x, y, str[i], color);
        cur_x += 8; // Смещение на ширину буквы
    }
}
