#ifndef VGA_H
#define VGA_H

#include <stddef.h>
#include <stdint.h>

// Цветовая палитра VGA
enum vga_color {
    BLACK = 0, BLUE = 1, GREEN = 2, CYAN = 3, RED = 4, MAGENTA = 5, BROWN = 6, LIGHT_GREY = 7,
    DARK_GREY = 8, LIGHT_BLUE = 9, LIGHT_GREEN = 10, LIGHT_CYAN = 11, LIGHT_RED = 12, PINK = 13, YELLOW = 14, WHITE = 15,
};

void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_putchar(char c);
void terminal_writestring(const char* data);
void terminal_clear(void);

#endif
