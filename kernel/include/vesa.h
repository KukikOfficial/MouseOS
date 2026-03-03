#ifndef VESA_H
#define VESA_H

#include <stdint.h>

void init_graphics(uint32_t* framebuffer);
void put_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void clear_screen(uint32_t color);

#endif
