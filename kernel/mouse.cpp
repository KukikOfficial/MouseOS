#include "include/io.h"
#include "include/vesa.h"

int mouse_x = 512, mouse_y = 384;
int old_mouse_x = 512, old_mouse_y = 384;

void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) { if ((inb(0x64) & 1) == 1) return; }
    } else {
        while (timeout--) { if ((inb(0x64) & 2) == 0) return; }
    }
}

void mouse_write(uint8_t write) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, write);
}

uint8_t mouse_read() {
    mouse_wait(0);
    return inb(0x60);
}

void init_mouse_driver() {
    uint8_t status;
    mouse_wait(1);
    outb(0x64, 0xA8); // Включить мышь
    mouse_wait(1);
    outb(0x64, 0x20); // Получить статус
    mouse_wait(0);
    status = (inb(0x60) | 2);
    mouse_wait(1);
    outb(0x64, 0x60); // Записать статус
    mouse_wait(1);
    outb(0x60, status);
    mouse_write(0xF4); // Включить передачу данных
}

void update_mouse_cursor() {
    // Стираем старый курсор (рисуем цветом фона)
    draw_rect(old_mouse_x, old_mouse_y, 8, 8, 0x1A2B3C);
    
    // Рисуем новый курсор (белый квадратик 8x8)
    draw_rect(mouse_x, mouse_y, 8, 8, 0xFFFFFF);
    
    old_mouse_x = mouse_x;
    old_mouse_y = mouse_y;
}
