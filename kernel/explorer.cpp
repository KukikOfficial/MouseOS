#include "include/fs.h"
#include "include/vesa.h"
#include "include/font.h"

// Ссылаемся на внешние данные
extern bool explorer_open; 
extern DiskFileEntry file_table[MAX_DISK_FILES];
extern "C" uint32_t get_file_count();

extern "C" void draw_explorer() {
    if (!explorer_open) return;

    // Рисуем окно проводника
    draw_rect(300, 200, 420, 320, 0xDDDDDD); // Основной фон
    draw_rect(300, 200, 420, 28, 0x444444);  // Заголовок
    draw_string(310, 208, "FILE EXPLORER", 0xFFFFFF);
    
    // Кнопка закрытия (крестик)
    draw_rect(695, 205, 20, 18, 0xAA0000);
    draw_string(702, 208, "X", 0xFFFFFF);

    uint32_t count = get_file_count();
    int cur_y = 240;

    if (count == 0) {
        draw_string(320, cur_y, "EMPTY DISK", 0x777777);
    } else {
        for (int i = 0; i < MAX_DISK_FILES; i++) {
            if (file_table[i].flags == 1) {
                draw_string(320, cur_y, "[FILE] ", 0x0000FF);
                draw_string(370, cur_y, file_table[i].name, 0x000000);
                cur_y += 20;
            }
        }
    }
}
