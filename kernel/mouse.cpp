#include "include/mouse.h"
#include "include/io.h"
#include "include/vesa.h"

int mouse_x = 512, mouse_y = 384;
int old_x = 512, old_y = 384;

uint8_t mouse_cycle = 0;
int8_t mouse_data[3]; // Массив для пакета данных

// Функция чтения байта напрямую из порта (с проверкой готовности)
uint8_t mouse_read_raw() {
    return inb(0x60);
}

extern "C" void mouse_handler_main() {
    uint8_t status = inb(0x64);
    
    // Проверяем, что данные пришли именно от мыши (бит 5 порта 0x64)
    if (!(status & 0x01) || !(status & 0x20)) {
        outb(0xA0, 0x20); outb(0x20, 0x20);
        return;
    }

    mouse_data[mouse_cycle++] = inb(0x60);

    if (mouse_cycle == 3) {
        mouse_cycle = 0;

        // ПРОВЕРКА ПЕРВОГО БАЙТА (биты 4 и 5 - знаки X и Y)
        // Если 4-й бит (0x10) установлен - движение X отрицательное
        // Если 5-й бит (0x20) установлен - движение Y отрицательное
        
        int rel_x = mouse_data[1];
        int rel_y = mouse_data[2];

        // Обработка знака X
        if (mouse_data[0] & 0x10) rel_x -= 256;
        // Обработка знака Y (инвертируем, так как в PS/2 вверх - это плюс, а в VESA - минус)
        if (mouse_data[0] & 0x20) rel_y -= 256;

        // Применяем движение к координатам Mouse OS
        mouse_x += rel_x;
        mouse_y -= rel_y; // Инвертируем Y для корректного движения "вверх-вниз"

        // Границы экрана 1024x768
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > 1023) mouse_x = 1023;
        if (mouse_y > 767) mouse_y = 767;

        update_mouse_cursor();
    }

    // Двойной EOI
    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void update_mouse_cursor() {
    // Стираем старый (фон 0x1A2B3C)
    draw_rect(old_x, old_y, 10, 10, 0x1A2B3C);
    
    // РИСУЕМ СТРЕЛОЧКУ (Mouse OS Style)
    // Рисуем вертикальную и горизонтальную палочки, чтобы получился уголок
    draw_rect(mouse_x, mouse_y, 2, 12, 0xFFFFFF); // Вертикаль
    draw_rect(mouse_x, mouse_y, 10, 2, 0xFFFFFF); // Горизонталь
    put_pixel(mouse_x + 2, mouse_y + 2, 0xFFFFFF);
    put_pixel(mouse_x + 3, mouse_y + 3, 0xFFFFFF);
    
    old_x = mouse_x;
    old_y = mouse_y;
}
