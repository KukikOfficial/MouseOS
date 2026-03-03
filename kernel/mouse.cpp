#include "include/mouse.h"
#include "include/io.h"
#include "include/vesa.h"

int mouse_x = 512, mouse_y = 384;
int old_x = 512, old_y = 384;
uint8_t mouse_cycle = 0;
int8_t mouse_data[3]; // Пакет из 3 байт
bool left_clicked = false;

extern "C" {

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

void init_mouse_driver() {
    uint8_t status;
    mouse_wait(1);
    outb(0x64, 0xA8); 
    mouse_wait(1);
    outb(0x64, 0x20); 
    mouse_wait(0);
    status = (inb(0x60) | 2);
    mouse_wait(1);
    outb(0x64, 0x60); 
    mouse_wait(1);
    outb(0x60, status);
    mouse_write(0xF4); 
}

void mouse_handler_main() {
    uint8_t status = inb(0x64);
    if (!(status & 0x01) || !(status & 0x20)) {
        outb(0xA0, 0x20); outb(0x20, 0x20);
        return;
    }

    uint8_t mouse_byte = inb(0x60);

    // Синхронизация по 3-му биту (должен быть 1)
    if (mouse_cycle == 0 && !(mouse_byte & 0x08)) {
        outb(0xA0, 0x20); outb(0x20, 0x20);
        return;
    }

    mouse_data[mouse_cycle++] = mouse_byte;

    if (mouse_cycle == 3) {
        mouse_cycle = 0;

        // ПРОВЕРКА КНОПОК (1-й байт, бит 0 - левая кнопка)
        left_clicked = (mouse_data[0] & 0x01);

        int rel_x = (int)mouse_data[1];
        int rel_y = (int)mouse_data[2];

        if (mouse_data[0] & 0x10) rel_x -= 256;
        if (mouse_data[0] & 0x20) rel_y -= 256;

        mouse_x += rel_x;
        mouse_y -= rel_y;

        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > 1010) mouse_x = 1010;
        if (mouse_y > 750) mouse_y = 750;

        update_mouse_cursor();
    }

    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void update_mouse_cursor() {
    // Стираем старый курсор
    draw_rect(old_x, old_y, 12, 12, 0x1A2B3C);
    
    // Если нажата левая кнопка - оставляем след (для теста)
    if (left_clicked) {
        draw_rect(mouse_x, mouse_y, 4, 4, 0xFFFF00); // Желтый след
    }

    // Рисуем курсор-уголок
    draw_rect(mouse_x, mouse_y, 2, 10, 0xFFFFFF);
    draw_rect(mouse_x, mouse_y, 10, 2, 0xFFFFFF);
    
    old_x = mouse_x;
    old_y = mouse_y;
}

} // extern "C"
