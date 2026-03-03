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

// --- РАБОТА СО ВРЕМЕНЕМ (RTC) ---
uint8_t get_rtc_register(int reg) {
    outb(0x70, reg);
    return inb(0x71);
}

int bcd_to_bin(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void draw_clock() {
    // Читаем часы и минуты из чипа CMOS
    int hours = bcd_to_bin(get_rtc_register(0x04));
    int mins  = bcd_to_bin(get_rtc_register(0x02));
    
    char time_str[6];
    time_str[0] = (hours / 10) + '0';
    time_str[1] = (hours % 10) + '0';
    time_str[2] = ':';
    time_str[3] = (mins / 10) + '0';
    time_str[4] = (mins % 10) + '0';
    time_str[5] = '\0';

    // Отрисовка в правом углу панели задач
    draw_string(950, 742, time_str, 0x00FF00); 
}

// --- ГРАФИКА И ИНТЕРФЕЙС ---
void redraw_interface_at(int x, int y) {
    // Стираем старую позицию курсора
    draw_rect(x, y, 14, 14, 0x1A2B3C); 

    // Восстановление окна Mouse Core
    if (x + 14 > 150 && x < 874 && y + 14 > 100 && y < 600) {
        draw_rect(150, 100, 724, 500, 0xCCCCCC);
        draw_rect(150, 100, 724, 32, 0x0055AA);
        draw_string(160, 108, "MOUSE OS CORE", 0xFFFFFF);
    }

    // Восстановление панели задач и часов
    if (y + 14 > 730) {
        draw_rect(0, 730, 1024, 38, 0x222222);
        draw_string(10, 742, "[ START ]", 0xFFFFFF);
        draw_clock(); // Часы перерисовываются при движении мыши
    }

    // Восстановление меню
    if (menu_open && x < 200 && y + 14 > 530 && y < 730) {
        draw_rect(0, 530, 200, 200, 0x444444);
        draw_string(10, 550, "> SYSTEM", 0xFFFFFF);
        draw_string(10, 580, "> REBOOT", 0xFFFFFF);
    }
}

// --- ДРАЙВЕР МЫШИ ---
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
    if (mouse_cycle == 0 && !(mouse_byte & 0x08)) {
        outb(0xA0, 0x20); outb(0x20, 0x20);
        return;
    }

    mouse_data[mouse_cycle++] = (int8_t)mouse_byte;

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

        // Клик по кнопке ПУСК
        if (left_clicked && mouse_x < 100 && mouse_y > 730) {
            menu_open = !menu_open;
            if (menu_open) {
                draw_rect(0, 530, 200, 200, 0x444444);
                draw_string(10, 550, "> SYSTEM", 0xFFFFFF);
                draw_string(10, 580, "> REBOOT", 0xFFFFFF);
            } else {
                draw_rect(0, 530, 200, 200, 0x1A2B3C); 
            }
        }
        
        // Клик по REBOOT
        if (left_clicked && menu_open && mouse_x < 200 && mouse_y > 575 && mouse_y < 595) {
            outb(0x64, 0xFE); 
        }

        update_mouse_cursor();
    }

    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void update_mouse_cursor() {
    redraw_interface_at(old_x, old_y);
    // Рисуем курсор
    draw_rect(mouse_x, mouse_y, 2, 12, 0xFFFFFF);
    draw_rect(mouse_x, mouse_y, 12, 2, 0xFFFFFF);
    old_x = mouse_x;
    old_y = mouse_y;
}

} // extern "C"
