#include "include/mouse.h"
#include "include/io.h"
#include "include/vesa.h"
#include "include/font.h"
#include "include/fs.h"

int mouse_x = 512, mouse_y = 384;
int old_x = 512, old_y = 384;
uint8_t mouse_cycle = 0;
int8_t mouse_data[3]; 
bool left_clicked = false;
bool menu_open = false;
bool explorer_open = false;

extern "C" {

// Работа с часами RTC
uint8_t get_rtc(int reg) { outb(0x70, reg); return inb(0x71); }
int bcd_to_bin(uint8_t bcd) { return ((bcd >> 4) * 10) + (bcd & 0x0F); }

void draw_clock() {
    int h = bcd_to_bin(get_rtc(0x04));
    int m = bcd_to_bin(get_rtc(0x02));
    char t[6] = {(char)(h/10+'0'), (char)(h%10+'0'), ':', (char)(m/10+'0'), (char)(m%10+'0'), 0};
    draw_string(960, 742, t, 0x00FF00);
}

// Перерисовка элементов интерфейса под курсором
void redraw_at(int x, int y) {
    draw_rect(x, y, 14, 14, 0x1A2B3C); // Фон

    // Окно консоли
    if (x+14 > 150 && x < 874 && y+14 > 100 && y < 600) {
        draw_rect(150, 100, 724, 500, 0xCCCCCC);
        draw_rect(150, 100, 724, 32, 0x0055AA);
        draw_string(160, 108, "Mouse OS Console", 0xFFFFFF);
    }

    // Проводник
    if (explorer_open && x+14 > 300 && x < 700 && y+14 > 200 && y < 500) {
        draw_rect(300, 200, 400, 300, 0xEEEEEE);
        draw_rect(300, 200, 400, 25, 0x333333);
        draw_string(310, 205, "File Explorer", 0xFFFFFF);
    }

    // Панель задач
    if (y+14 > 730) {
        draw_rect(0, 730, 1024, 38, 0x222222);
        draw_string(10, 742, "[ START ]", 0xFFFFFF);
        draw_clock();
    }

    // Меню ПУСК
    if (menu_open && x < 200 && y+14 > 530 && y < 730) {
        draw_rect(0, 530, 200, 200, 0x444444);
        draw_string(10, 550, "> FILES", 0xFFFFFF);
        draw_string(10, 580, "> REBOOT", 0xFFFFFF);
    }
}

void mouse_handler_main() {
    uint8_t status = inb(0x64);
    if (!(status & 0x01) || !(status & 0x20)) { outb(0xA0, 0x20); outb(0x20, 0x20); return; }

    uint8_t b = inb(0x60);
    if (mouse_cycle == 0 && !(b & 0x08)) { outb(0xA0, 0x20); outb(0x20, 0x20); return; }

    mouse_data[mouse_cycle++] = b;

    if (mouse_cycle == 3) {
        mouse_cycle = 0;
        left_clicked = (mouse_data[0] & 0x01);

        int32_t rel_x = (int32_t)mouse_data[1];
        int32_t rel_y = (int32_t)mouse_data[2];
        if (mouse_data[0] & 0x10) rel_x |= 0xFFFFFF00;
        if (mouse_data[0] & 0x20) rel_y |= 0xFFFFFF00;

        mouse_x += rel_x; mouse_y -= rel_y;
        if (mouse_x < 0) mouse_x = 0; if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > 1010) mouse_x = 1010; if (mouse_y > 754) mouse_y = 754;

        // Обработка клика Пуск
        if (left_clicked && mouse_x < 100 && mouse_y > 730) {
            menu_open = !menu_open;
            clear_screen(0x1A2B3C); // Форсированный сброс для чистоты
        }

        // Клик по FILES в меню
        if (left_clicked && menu_open && mouse_x < 200 && mouse_y > 540 && mouse_y < 570) {
            explorer_open = !explorer_open;
            menu_open = false;
            clear_screen(0x1A2B3C);
        }

        // Клик по REBOOT
        if (left_clicked && menu_open && mouse_x < 200 && mouse_y > 575 && mouse_y < 600) {
            outb(0x64, 0xFE);
        }

        redraw_at(old_x, old_y);
        draw_rect(mouse_x, mouse_y, 2, 12, 0xFFFFFF);
        draw_rect(mouse_x, mouse_y, 12, 2, 0xFFFFFF);
        old_x = mouse_x; old_y = mouse_y;
    }
    outb(0xA0, 0x20); outb(0x20, 0x20);
}

void init_mouse_driver() {
    uint8_t status;
    outb(0x64, 0xA8); outb(0x64, 0x20);
    status = (inb(0x60) | 2);
    outb(0x64, 0x60); outb(0x60, status);
    outb(0x64, 0xD4); outb(0x60, 0xF4);
}

} // extern "C"
