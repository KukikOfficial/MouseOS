#include "include/fs.h"
#include "include/vesa.h"
#include "include/font.h"
#include "include/process.h"

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

    // 1. Основное окно проводника
    draw_rect(explorer_x, explorer_y, 420, 320, 0xDDDDDD); 
    draw_rect(explorer_x, explorer_y, 420, 28, 0x444444);  // Заголовок (Зона drag-and-drop)
    draw_string(explorer_x + 10, explorer_y + 8, "FILE EXPLORER", 0xFFFFFF);
    
    // Кнопка закрытия X
    draw_rect(explorer_x + 395, explorer_y + 5, 20, 18, 0xAA0000);
    draw_string(explorer_x + 402, explorer_y + 8, "X", 0xFFFFFF);

    // 2. ИСПРАВЛЕНО: Полоса прокрутки (Scrollbar) теперь имеет ширину 25px и прижата к правому краю окна
    int sb_x = explorer_x + 395;
    int sb_y = explorer_y + 28; // Начинается сразу под заголовком
    int sb_w = 25;
    int sb_h = 292;             // Идет до самого низа окна (28 + 292 = 320)
    draw_rect(sb_x, sb_y, sb_w, sb_h, 0xBBBBBB); // Трэк скроллбара

    // Динамический расчет максимума прокрутки
    uint32_t total_files = get_file_count();
    int max_scroll = (total_files > 9) ? (total_files - 9) : 0;

    // Отрисовка ползунка (индикатора скролла)
    int slider_h = 40;
    int slider_y = sb_y + 5;
    if (max_scroll > 0) {
        // Положение ползунка динамически распределяется по высоте полосы
        slider_y = sb_y + 5 + (explorer_scroll * (sb_h - slider_h - 10) / max_scroll);
    }
    draw_rect(sb_x + 2, slider_y, sb_w - 4, slider_h, 0x777777);

    // 3. Фиксированная кнопка создания файла
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
                if (skipped_files < explorer_scroll) {
                    skipped_files++;
                    continue;
                }

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

    draw_rect(tasks_x, tasks_y, 300, 250, 0xEEEEEE); 
    draw_rect(tasks_x, tasks_y, 300, 28, 0x555555); 
    draw_string(tasks_x + 10, tasks_y + 8, "TASK MANAGER", 0xFFFFFF);

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