#include "include/vga.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = WHITE;
    terminal_buffer = (uint16_t*) 0xB8000;
    terminal_clear();
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = (uint16_t) ' ' | (uint16_t) terminal_color << 8;
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}

void terminal_putchar(char c) {
    // Обработка Backspace (стирание)
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
        } else if (terminal_row > 0) {
            terminal_row--;
            terminal_column = VGA_WIDTH - 1;
        }
        terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] = (uint16_t) ' ' | (uint16_t) terminal_color << 8;
        return;
    }

    // Перенос строки
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) terminal_clear();
        return;
    }

    // Печать обычного символа
    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    terminal_buffer[index] = (uint16_t) c | (uint16_t) terminal_color << 8;
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) terminal_clear();
    }
}

void terminal_writestring(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) terminal_putchar(data[i]);
}
