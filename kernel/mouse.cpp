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

void draw_explorer(); 

uint8_t get_rtc(int reg) { outb(0x70, reg); return inb(0x71); }
int bcd_to_bin(uint8_t bcd) { return ((bcd >> 4) * 10) + (bcd & 0x0F); }

void draw_clock() {
    int h = bcd_to_bin(get_rtc(0x04));
    int m = bcd_to_bin(get_rtc(0x02));
    char t[6];
    t[0] = (h/10) + '0'; t[1] = (h%10) + '0'; t[2] = ':';
    t[3] = (m/10) + '0'; t[4] = (m%10) + '0'; t[5] = '\0';
    draw_string(960, 742, t, 0x00FF00);
}

// Рисует весь статический UI в фоновый буфер backbuffer
void draw_all_ui() {
    clear_screen(0x1A2B3C); // Чистый красивый фон рабочего стола БЕЗ лишних окон!

    // Если открыт проводник — рисуем его поверх
    if (explorer_open) {
        draw_explorer(); 
    }

    // Нижняя панель задач
    draw_rect(0, 730, 1024, 38, 0x222222);
    draw_string(10, 742, "[ START ]", 0xFFFFFF);
    draw_clock();

    if (menu_open) {
        draw_rect(0, 530, 200, 200, 0x444444);
        draw_string(10, 550, "> FILES", 0xFFFFFF);
        draw_string(10, 580, "> REBOOT", 0xFFFFFF);
    }
    
    update_screen();
}

void redraw_interface_at(int x, int y) {
    draw_rect(x, y, 16, 16, 0x1A2B3C); // Стираем старый курсор цветом фона рабочего стола

    // Проверяем пересечение с проводником
    if (explorer_open && x+16 > 300 && x < 720 && y+16 > 200 && y < 520) {
        draw_explorer();
    }

    // Проверяем пересечение с панелью задач
    if (y+16 > 730) {
        draw_rect(0, 730, 1024, 38, 0x222222);
        draw_string(10, 742, "[ START ]", 0xFFFFFF);
        draw_clock();
    }

    // Проверяем пересечение с меню ПУСК
    if (menu_open && x < 200 && y+16 > 530 && y < 730) {
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

        if (mouse_x < 0) mouse_x = 0; 
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > 1010) mouse_x = 1010; 
        if (mouse_y > 754) mouse_y = 754;

        if (left_clicked) {
            if (mouse_x < 100 && mouse_y > 730) {
                menu_open = !menu_open;
                draw_all_ui(); // Полное обновление статического кадра
            }
            else if (menu_open && mouse_x < 200 && mouse_y > 540 && mouse_y < 570) {
                explorer_open = !explorer_open;
                menu_open = false;
                draw_all_ui(); 
            }
            else if (menu_open && mouse_x < 200 && mouse_y > 575 && mouse_y < 605) {
                outb(0x64, 0xFE);
            }
            else if (explorer_open && mouse_x > 695 && mouse_x < 715 && mouse_y > 205 && mouse_y < 225) {
                explorer_open = false;
                draw_all_ui(); 
            }
        }

        update_mouse_cursor();
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

// ИСПРАВЛЕНО: Ультра-быстрое обновление позиции курсора БЕЗ мерцания экрана
void update_mouse_cursor() {
    // 1. Копируем чистый UI из памяти на экран (автоматически затирает мышь на старом месте)
    update_screen();
    
    // 2. Рисуем новый курсор мыши НАПРЯМУЮ на экран поверх картинки, не загрязняя фоновый буфер
    draw_rect_front(mouse_x, mouse_y, 2, 12, 0xFFFFFF);
    draw_rect_front(mouse_x, mouse_y, 12, 2, 0xFFFFFF);
    
    old_x = mouse_x; old_y = mouse_y;
}

} // extern "C"