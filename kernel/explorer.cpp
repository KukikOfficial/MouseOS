#include "include/fs.h"
#include "include/vesa.h"
#include "include/font.h"

bool explorer_open = false;

void draw_explorer() {
    if (!explorer_open) return;

    // Окно проводника
    draw_rect(300, 200, 400, 300, 0xDDDDDD); // Фон
    draw_rect(300, 200, 400, 25, 0x555555);  // Заголовок
    draw_string(310, 208, "File Explorer", 0xFFFFFF);

    // Список файлов из нашей MouseFS
    int start_y = 240;
    // Используем глобальную переменную из fs.cpp (file_table)
    extern DiskFileEntry file_table[16];
    extern uint32_t disk_file_count; // Нужно добавить в fs.cpp

    for (int i = 0; i < 16; i++) {
        if (file_table[i].flags == 1) {
            draw_string(320, start_y, "[FILE] ", 0x0000FF);
            draw_string(370, start_y, file_table[i].name, 0x000000);
            start_y += 20;
        }
    }
}
