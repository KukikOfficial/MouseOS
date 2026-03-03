#include "include/mouse.h"
#include "include/io.h"
#include "include/vesa.h"
#include "include/font.h"

int mouse_x = 512, mouse_y = 384;
int old_x = 512, old_y = 384;
uint8_t mouse_cycle = 0;
int8_t mouse_data[3]; 
bool left_clicked = false;
bool menu_open = false;

extern "C" {

// Функция восстановления графики под старым курсором
void redraw_interface_at(int x, int y) {
    // 1. Сначала рисуем чистый фон рабочего стола
    draw_rect(x, y, 14, 14, 0x1A2B3C); 

    // 2. Если мы задели главное окно (150, 100, 724, 500)
    if (x + 14 > 150 && x < 874 && y + 14 > 100 && y < 600) {
        draw_rect(150, 100, 724, 500, 0xCCCCCC); // Тело
        draw_rect(150, 100, 724, 32, 0x0055AA);  // Заголовок
        draw_string(160, 108, "MOUSE OS CORE", 0xFFFFFF);
    }

    // 3. Если задели панель задач (внизу)
    if (y + 14 > 730) {
        draw_rect(0, 730, 1024, 38, 0x222222);
        draw_string(10, 742, "[ START ]", 0xFFFFFF);
    }

    // 4. Если задели открытое меню
    if (menu_open && x < 200 && y + 14 > 530 && y < 730) {
        draw_rect(0, 530, 200, 200, 0x444444);
        draw_string(10, 550, "> SYSTEM", 0xFFFFFF);
        draw_string(10, 580, "> REBOOT", 0xFFFFFF);
    }
}

void mouse_handler_main() {
    uint8_t status = inb(0x64);
    if (!(status & 0x01) || !(status & 0x20)) {
        outb(0xA0, 0x20); outb(0x20, 0x20);
        return;
    }

    uint8_t mouse_byte = inb(0x60);
    if (mouse_cycle == 0 && !(mouse_byte & 0x08)) {
        outb(0xA0, 0x20); outb(0x20, 0x20);
        return;
    }

    mouse_data[mouse_cycle++] = mouse_byte;

    if (mouse_cycle == 3) {
        mouse_cycle = 0;
        left_clicked = (mouse_data[0] & 0x01);

        int32_t rel_x = (int32_t)mouse_data[1];
        int32_t rel_y = (int32_t)mouse_data[2];

        if (mouse_data[0] & 0x10) rel_x |= 0xFFFFFF00;
        if (mouse_data[0] & 0x20) rel_y |= 0xFFFFFF00;

        mouse_x += rel_x;
        mouse_y -= rel_y;

        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > 1010) mouse_x = 1010;
        if (mouse_y > 754) mouse_y = 754;

        // Логика клика по меню ПУСК
        if (left_clicked && mouse_x < 100 && mouse_y > 730) {
            menu_open = !menu_open;
            // Рисуем меню сразу (форсированно)
            if (menu_open) {
                draw_rect(0, 530, 200, 200, 0x444444);
                draw_string(10, 550, "> SYSTEM", 0xFFFFFF);
                draw_string(10, 580, "> REBOOT", 0xFFFFFF);
            } else {
                draw_rect(0, 530, 200, 200, 0x1A2B3C); // Стираем меню
            }
        }

        update_mouse_cursor();
    }

    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void update_mouse_cursor() {
    redraw_interface_at(old_x, old_y);
    
    // Рисуем белый уголок-курсор
    draw_rect(mouse_x, mouse_y, 2, 12, 0xFFFFFF);
    draw_rect(mouse_x, mouse_y, 12, 2, 0xFFFFFF);
    
    old_x = mouse_x;
    old_y = mouse_y;
}

} // extern "C"
