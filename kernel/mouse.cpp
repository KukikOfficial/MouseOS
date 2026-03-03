#include "include/mouse.h"
#include "include/io.h"
#include "include/vesa.h"

int mouse_x = 512, mouse_y = 384;
int old_x = 512, old_y = 384;
uint8_t mouse_cycle = 0;
int8_t mouse_data[3]; // Пакет из 3 байт

extern "C" {

void mouse_handler_main() {
    uint8_t status = inb(0x64);
    
    // 1. Проверяем, есть ли данные и от мыши ли они
    if (!(status & 0x01) || !(status & 0x20)) {
        outb(0xA0, 0x20); outb(0x20, 0x20);
        return;
    }

    uint8_t mouse_byte = inb(0x60);

    // 2. СИНХРОНИЗАЦИЯ: Первый байт пакета ВСЕГДА имеет 3-й бит = 1
    // Если мы на первом байте и 3-й бит равен 0 — значит мы "потерялись", игнорируем байт
    if (mouse_cycle == 0 && !(mouse_byte & 0x08)) {
        outb(0xA0, 0x20); outb(0x20, 0x20);
        return;
    }

    mouse_data[mouse_cycle++] = mouse_byte;

    if (mouse_cycle == 3) {
        mouse_cycle = 0;

        // Обработка относительного движения
        int rel_x = (int)mouse_data[1];
        int rel_y = (int)mouse_data[2];

        // Учитываем биты знака из первого байта
        if (mouse_data[0] & 0x10) rel_x -= 256;
        if (mouse_data[0] & 0x20) rel_y -= 256;

        // Обновляем координаты Mouse OS
        mouse_x += rel_x;
        mouse_y -= rel_y; // Инверсия для экрана

        // Ограничители (чтобы курсор не убегал за экран)
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > 1014) mouse_x = 1014;
        if (mouse_y > 758) mouse_y = 758;

        update_mouse_cursor();
    }

    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void update_mouse_cursor() {
    // Стираем старый (фон Mouse OS: 0x1A2B3C)
    // Рисуем квадрат чуть больше, чтобы точно затереть хвосты
    draw_rect(old_x, old_y, 12, 12, 0x1A2B3C);
    
    // Рисуем новый курсор (белый уголок)
    draw_rect(mouse_x, mouse_y, 2, 10, 0xFFFFFF);
    draw_rect(mouse_x, mouse_y, 10, 2, 0xFFFFFF);
    
    old_x = mouse_x;
    old_y = mouse_y;
}

} // extern "C"
