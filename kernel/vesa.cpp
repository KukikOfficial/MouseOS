#include "include/vesa.h"

uint32_t* lfb; // Linear Framebuffer
const int WIDTH = 1024;
const int HEIGHT = 768;

void init_graphics(uint32_t* framebuffer_addr) {
    lfb = framebuffer_addr;
}

void put_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        lfb[y * WIDTH + x] = color;
    }
}

void clear_screen(uint32_t color) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        lfb[i] = color;
    }
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            put_pixel(x + j, y + i, color);
        }
    }
}
