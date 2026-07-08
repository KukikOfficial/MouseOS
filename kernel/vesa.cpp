#include "include/vesa.h"
#include "include/memory.h" // Для использования kmalloc

uint32_t* lfb;         // Физическая видеопамять (экран)
uint32_t* backbuffer;  // Наш скрытый буфер в RAM
const int WIDTH = 1024;
const int HEIGHT = 768;

void init_graphics(uint32_t* framebuffer_addr) {
    lfb = framebuffer_addr;
    // Выделяем память под буфер экрана (1024 * 768 * 4 байта = 3 МБ)
    backbuffer = (uint32_t*)kmalloc(WIDTH * HEIGHT * sizeof(uint32_t));
}

// Рисует в фоновый буфер (не вызывает мерцания)
void put_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        backbuffer[y * WIDTH + x] = color;
    }
}

// Очистка фонового буфера
void clear_screen(uint32_t color) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        backbuffer[i] = color;
    }
}

// Рисование прямоугольника в фоновый буфер
void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            put_pixel(x + j, y + i, color);
        }
    }
}

// Мгновенно копирует весь готовый фоновый буфер на физический экран
void update_screen() {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        lfb[i] = backbuffer[i];
    }
}

// Рисует прямоугольник НАПРЯМУЮ на экран (используется ТОЛЬКО для курсора мыши)
void draw_rect_front(int x, int y, int w, int h, uint32_t color) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int cur_x = x + j;
            int cur_y = y + i;
            if (cur_x >= 0 && cur_x < WIDTH && cur_y >= 0 && cur_y < HEIGHT) {
                lfb[cur_y * WIDTH + cur_x] = color;
            }
        }
    }
}