#include "include/mouse.h"
#include "include/io.h"
#include "include/vesa.h"

// Координаты мыши Mouse OS
int mouse_x = 512;
int mouse_y = 384;
int old_x = 512;
int old_y = 384;

uint8_t mouse_cycle = 0;
int8_t mouse_byte[3];

// Ждем готовности контроллера
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

// ТОТ САМЫЙ ОБРАБОТЧИК, который искал линковщик
extern "C" void mouse_handler_main() {
    uint8_t status = inb(0x64);
    if (!(status & 1)) {
        outb(0xA0, 0x20); outb(0x20, 0x20);
        return;
    }

    mouse_byte[mouse_cycle++] = inb(0x60);

    if (mouse_cycle == 3) {
        mouse_cycle = 0;
        // Обработка движения
        mouse_x += mouse_byte[1];
        mouse_y -= mouse_byte[2];

        // Ограничители экрана 1024x768
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > 1023) mouse_x = 1023;
        if (mouse_y > 767) mouse_y = 767;
        
        update_mouse_cursor();
    }

    // Двойной EOI для Slave и Master PIC
    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void update_mouse_cursor() {
    // Стираем старый (рисуем цветом фона)
    draw_rect(old_x, old_y, 8, 8, 0x1A2B3C);
    // Рисуем новый (белый квадрат)
    draw_rect(mouse_x, mouse_y, 8, 8, 0xFFFFFF);
    
    old_x = mouse_x;
    old_y = mouse_y;
}
