#include "include/fs.h"
#include "include/vesa.h"
#include "include/font.h"
#include "include/process.h"

// Ссылаемся на динамические координаты окон и состояние скролла из mouse.cpp
extern int explorer_x, explorer_y;
extern int tasks_x, tasks_y;
extern int explorer_scroll;

extern bool explorer_open; 
extern bool tasks_open;
extern DiskFileEntry file_table[MAX_DISK_FILES];
extern Process process_table[MAX_PROCESSES];

extern "C" {

uint32_t get_file_count();

void draw_explorer() {
    if (!explorer_open) return;

    // 1. Рисуем основное окно проводника по динамическим координатам
    draw_rect(explorer_x, explorer_y, 420, 320, 0xDDDDDD); // Основной фон
    draw_rect(explorer_x, explorer_y, 420, 28, 0x444444);  // Заголовок (Зона перетаскивания)
    draw_string(explorer_x + 10, explorer_y + 8, "FILE EXPLORER", 0xFFFFFF);
    
    // Кнопка закрытия (крестик) привязана к правому верхнему углу окна
    draw_rect(explorer_x + 395, explorer_y + 5, 20, 18, 0xAA0000);
    draw_string(explorer_x + 402, explorer_y + 8, "X", 0xFFFFFF);

    // 2. Отрисовка полосы прокрутки (Scrollbar / Slider) справа
    int sb_x = explorer_x + 395;
    int sb_y = explorer_y + 35;
    int sb_w = 15;
    int sb_h = 240;
    draw_rect(sb_x, sb_y, sb_w, sb_h, 0xBBBBBB); // Трек слайдера

    // Визуальный ползунок, положение которого зависит от explorer_scroll
    // Всего файлов максимум 16, на экране помещается 9. Макс скролл = 7 уровней.
    int slider_block_h = 40;
    int slider_block_y = sb_y + (explorer_scroll * 25); 
    draw_rect(sb_x + 1, slider_block_y, sb_w - 2, slider_block_h, 0x777777); // Сам ползунок

    // 3. Фиксированная кнопка создания файла внизу окна (больше не перекрывается файлами!)
    draw_rect(explorer_x + 15, explorer_y + 285, 100, 24, 0x00AA55);
    draw_string(explorer_x + 25, explorer_y + 293, "NEW FILE", 0xFFFFFF);

    uint32_t count = get_file_count();
    int cur_y = explorer_y + 40;

    if (count == 0) {
        draw_string(explorer_x + 20, cur_y, "EMPTY DISK", 0x777777);
    } else {
        int visible_count = 0;
        int skipped_files = 0;

        for (int i = 0; i < MAX_DISK_FILES; i++) {
            if (file_table[i].flags == 1) {
                // Пропускаем файлы, которые остались сверху при прокрутке
                if (skipped_files < explorer_scroll) {
                    skipped_files++;
                    continue;
                }

                // Выводим строго не больше 9 файлов в область видимости окна
                if (visible_count < 9) {
                    draw_string(explorer_x + 20, cur_y, "[FILE] ", 0x0000FF);
                    draw_string(explorer_x + 80, cur_y, file_table[i].name, 0x000000);
                    cur_y += 24;
                    visible_count++;
                }
            }
        }
    }
}

void draw_task_manager() {
    if (!tasks_open) return;

    // Рисуем Диспетчер задач по динамическим координатам
    draw_rect(tasks_x, tasks_y, 300, 250, 0xEEEEEE); 
    draw_rect(tasks_x, tasks_y, 300, 28, 0x555555);  // Заголовок (Зона перетаскивания)
    draw_string(tasks_x + 10, tasks_y + 8, "TASK MANAGER", 0xFFFFFF);

    // Кнопка закрытия
    draw_rect(tasks_x + 275, tasks_y + 5, 20, 18, 0xAA0000);
    draw_string(tasks_x + 282, tasks_y + 8, "X", 0xFFFFFF);

    int cur_y = tasks_y + 40;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != PROCESS_STOPPED) {
            char pid_str[8] = "PID: 0 ";
            pid_str[5] = '0' + (process_table[i].pid % 10);

            draw_string(tasks_x + 15, cur_y, pid_str, 0x0000FF);
            draw_string(tasks_x + 75, cur_y, process_table[i].name, 0x000000);
            cur_y += 22;
        }
    }
}

} // extern "C"