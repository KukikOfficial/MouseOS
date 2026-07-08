#include "include/mouse.h"
#include "include/io.h"
#include "include/vesa.h"
#include "include/font.h"
#include "include/fs.h"
#include "include/process.h" // Подключаем процессы

int mouse_x = 512, mouse_y = 384;
int old_x = 512, old_y = 384;
uint8_t mouse_cycle = 0;
int8_t mouse_data[3]; 
bool left_clicked = false;
bool menu_open = false;
bool explorer_open = false;
bool tasks_open = false; // Состояние окна диспетчера задач

extern "C" {

void draw_explorer(); 
void draw_task_manager(); // Из explorer.cpp
uint32_t get_file_count();

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

void draw_all_ui() {
    clear_screen(0x1A2B3C); // ИСПРАВЛЕНО: Теперь тут чистый красивый рабочий стол без багнутых окон!

    if (explorer_open) draw_explorer(); 
    if (tasks_open) draw_task_manager(); // Рисуем диспетчер задач, если он запущен

    // Панель задач
    draw_rect(0, 730, 1024, 38, 0x222222);
    draw_string(10, 742, "[ START ]", 0xFFFFFF);
    draw_clock();

    // Увеличенное меню ПУСК под 3 элемента
    if (menu_open) {
        draw_rect(0, 490, 200, 240, 0x444444);
        draw_string(10, 510, "> FILES", 0xFFFFFF);
        draw_string(10, 550, "> TASKS", 0xFFFFFF); // Новый пункт меню
        draw_string(10, 590, "> REBOOT", 0xFFFFFF);
    }
    
    update_screen();
}

void redraw_interface_at(int x, int y) {
    draw_rect(x, y, 16, 16, 0x1A2B3C); 

    if (explorer_open && x+16 > 300 && x < 720 && y+16 > 200 && y < 520) draw_explorer();
    if (tasks_open && x+16 > 450 && x < 750 && y+16 > 250 && y < 500) draw_task_manager();

    if (y+16 > 730) {
        draw_rect(0, 730, 1024, 38, 0x222222);
        draw_string(10, 742, "[ START ]", 0xFFFFFF);
        draw_clock();
    }

    if (menu_open && x < 200 && y+16 > 490 && y < 730) {
        draw_rect(0, 490, 200, 240, 0x444444);
        draw_string(10, 510, "> FILES", 0xFFFFFF);
        draw_string(10, 550, "> TASKS", 0xFFFFFF);
        draw_string(10, 590, "> REBOOT", 0xFFFFFF);
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
            // Клик по ПУСК
            if (mouse_x < 100 && mouse_y > 730) {
                menu_open = !menu_open;
                draw_all_ui(); 
            }
            // Клик по FILES
            else if (menu_open && mouse_x < 200 && mouse_y > 500 && mouse_y < 535) {
                explorer_open = !explorer_open;
                menu_open = false;
                if (explorer_open) create_process("EXPLORER"); // Регистрируем процесс
                else terminate_process_by_name("EXPLORER");
                draw_all_ui(); 
            }
            // Клик по TASKS
            else if (menu_open && mouse_x < 200 && mouse_y > 540 && mouse_y < 575) {
                tasks_open = !tasks_open;
                menu_open = false;
                if (tasks_open) create_process("TASK_MGR");
                else terminate_process_by_name("TASK_MGR");
                draw_all_ui();
            }
            // Клик по REBOOT
            else if (menu_open && mouse_x < 200 && mouse_y > 580 && mouse_y < 615) {
                outb(0x64, 0xFE);
            }
            // Крестик закрытия Проводника
            else if (explorer_open && mouse_x > 695 && mouse_x < 715 && mouse_y > 205 && mouse_y < 225) {
                explorer_open = false;
                terminate_process_by_name("EXPLORER"); // Завершаем процесс
                draw_all_ui(); 
            }
            // Крестик закрытия Диспетчера задач
            else if (tasks_open && mouse_x > 725 && mouse_x < 745 && mouse_y > 255 && mouse_y < 275) {
                tasks_open = false;
                terminate_process_by_name("TASK_MGR");
                draw_all_ui();
            }
            // ИСПРАВЛЕНО: Клик по кнопке "NEW FILE" внутри проводника
            else if (explorer_open && mouse_x > 315 && mouse_x < 415 && mouse_y > 485 && mouse_y < 510) {
                uint32_t f_idx = get_file_count() + 1;
                char f_name[12] = "FILE0.TXT";
                f_name[4] = '0' + (f_idx % 10); // Динамически создаем имя FILE1.TXT, FILE2.TXT
                
                write_to_disk(f_name, "CREATED VIA GRAPHICAL UI");
                draw_all_ui(); // Мгновенно обновляем интерфейс, чтобы файл появился в списке!
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

void update_mouse_cursor() {
    update_screen();
    draw_rect_front(mouse_x, mouse_y, 2, 12, 0xFFFFFF);
    draw_rect_front(mouse_x, mouse_y, 12, 2, 0xFFFFFF);
    old_x = mouse_x; old_y = mouse_y;
}

} // extern "C"