#include "include/mouse.h"
#include "include/io.h"
#include "include/vesa.h"
#include "include/font.h"
#include "include/fs.h"
#include "include/process.h"

// Глобальные динамические координаты окон на рабочем столе
int explorer_x = 300, explorer_y = 200;
int tasks_x = 450, tasks_y = 250;

// Состояние прокрутки Проводника (сколько файлов скрыто сверху)
int explorer_scroll = 0;

int mouse_x = 512, mouse_y = 384;
int old_x = 512, old_y = 384;
uint8_t mouse_cycle = 0;
int8_t mouse_data[3]; 

bool left_clicked = false;
static bool last_left_state = false; // Для отслеживания момента нажатия кнопки

// Флаги перетаскивания окон мышью
bool drag_explorer = false;
bool drag_tasks = false;
int drag_off_x = 0;
int drag_off_y = 0;

bool menu_open = false;
bool explorer_open = false;
bool tasks_open = false; 

extern "C" {

void draw_explorer(); 
void draw_task_manager(); 
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
    clear_screen(0x1A2B3C); 

    if (explorer_open) draw_explorer(); 
    if (tasks_open) draw_task_manager(); 

    // Панель задач
    draw_rect(0, 730, 1024, 38, 0x222222);
    draw_string(10, 742, "[ START ]", 0xFFFFFF);
    draw_clock();

    if (menu_open) {
        draw_rect(0, 490, 200, 240, 0x444444);
        draw_string(10, 510, "> FILES", 0xFFFFFF);
        draw_string(10, 550, "> TASKS", 0xFFFFFF);
        draw_string(10, 590, "> REBOOT", 0xFFFFFF);
    }
    update_screen();
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

        // Вычисляем, был ли совершен ОДИНОЧНЫЙ клик (кнопка нажата именно в этот микромомент)
        bool click_pressed = (left_clicked && !last_left_state);
        last_left_state = left_clicked;

        // --- ЛОГИКА ПЕРЕТАСКИВАНИЯ ОКН (DRAG & DROP) ---
        if (left_clicked) {
            // Если мы еще ничего не тащим, проверяем, попал ли курсор на заголовки окон
            if (!drag_explorer && !drag_tasks) {
                // Проверяем заголовок Проводника (без учета зоны кнопки закрытия X)
                if (explorer_open && mouse_x >= explorer_x && mouse_x <= (explorer_x + 390) &&
                    mouse_y >= explorer_y && mouse_y <= (explorer_y + 28)) {
                    drag_explorer = true;
                    drag_off_x = mouse_x - explorer_x;
                    drag_off_y = mouse_y - explorer_y;
                }
                // Проверяем заголовок Диспетчера задач
                else if (tasks_open && mouse_x >= tasks_x && mouse_x <= (tasks_x + 270) &&
                         mouse_y >= tasks_y && mouse_y <= (tasks_y + 28)) {
                    drag_tasks = true;
                    drag_off_x = mouse_x - tasks_x;
                    drag_off_y = mouse_y - tasks_y;
                }
            }

            // Если захват окна активен, обновляем координаты вслед за курсором
            if (drag_explorer) {
                explorer_x = mouse_x - drag_off_x;
                explorer_y = mouse_y - drag_off_y;
                draw_all_ui(); 
            } else if (drag_tasks) {
                tasks_x = mouse_x - drag_off_x;
                tasks_y = mouse_y - drag_off_y;
                draw_all_ui();
            }
        } else {
            // Как только отпустили левую кнопку мыши — сбрасываем состояние перетаскивания
            drag_explorer = false;
            drag_tasks = false;
        }

        // --- ЛОГИКА ОБРАБОТКИ ОДИНОЧНЫХ КЛИКОВ ПО КНОПКАМ ---
        if (click_pressed) {
            // Клик по кнопке ПУСК
            if (mouse_x < 100 && mouse_y > 730) {
                menu_open = !menu_open;
                draw_all_ui(); 
            }
            // Клик по FILES в меню
            else if (menu_open && mouse_x < 200 && mouse_y > 500 && mouse_y < 535) {
                explorer_open = !explorer_open;
                menu_open = false;
                if (explorer_open) create_process("EXPLORER");
                else terminate_process_by_name("EXPLORER");
                draw_all_ui(); 
            }
            // Клик по TASKS в меню
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
            else if (explorer_open && mouse_x > (explorer_x + 395) && mouse_x < (explorer_x + 415) && 
                     mouse_y > (explorer_y + 5) && mouse_y < (explorer_y + 25)) {
                explorer_open = false;
                terminate_process_by_name("EXPLORER");
                draw_all_ui(); 
            }
            // Крестик закрытия Диспетчера задач
            else if (tasks_open && mouse_x > (tasks_x + 275) && mouse_x < (tasks_x + 295) && 
                     mouse_y > (tasks_y + 5) && mouse_y < (tasks_y + 25)) {
                tasks_open = false;
                terminate_process_by_name("TASK_MGR");
                draw_all_ui();
            }
            // Клик по фиксированной кнопке "NEW FILE"
            else if (explorer_open && mouse_x > (explorer_x + 15) && mouse_x < (explorer_x + 115) && 
                     mouse_y > (explorer_y + 285) && mouse_y < (explorer_y + 310)) {
                uint32_t f_idx = get_file_count() + 1;
                char f_name[12] = "FILE0.TXT";
                f_name[4] = '0' + (f_idx % 10); 
                
                write_to_disk(f_name, "CREATED VIA GRAPHICAL UI");
                draw_all_ui(); 
            }
            // --- НОВАЯ ЛОГИКА: Слайдер / Скроллинг в Проводнике ---
            else if (explorer_open && mouse_x > (explorer_x + 395) && mouse_x < (explorer_x + 410) && 
                     mouse_y > (explorer_y + 35) && mouse_y < (explorer_y + 275)) {
                
                // Нажимаем на верхнюю половину трека слайдера -> скролл вверх
                if (mouse_y < (explorer_y + 155)) {
                    if (explorer_scroll > 0) explorer_scroll--;
                } 
                // Нажимаем на нижнюю половину трека слайдера -> скролл вниз
                else {
                    // Максимальное смещение = 7 (так как файлов 16, а влазит 9: 16 - 9 = 7)
                    if (explorer_scroll < 7) explorer_scroll++;
                }
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

void update_mouse_cursor() {
    update_screen();
    draw_rect_front(mouse_x, mouse_y, 2, 12, 0xFFFFFF);
    draw_rect_front(mouse_x, mouse_y, 12, 2, 0xFFFFFF);
    old_x = mouse_x; old_y = mouse_y;
}

} // extern "C"