#include "include/fs.h"
#include "include/vesa.h"
#include "include/font.h"
#include "include/process.h"

extern int explorer_x, explorer_y;
extern int tasks_x, tasks_y;
extern int notepad_x, notepad_y; // Координаты блокнота
extern int explorer_scroll;

extern bool explorer_open; 
extern bool tasks_open;
extern bool notepad_open;        // Состояние окна блокнота
extern char current_editing_file[20];
extern char notepad_buffer[256];

extern DiskFileEntry file_table[MAX_DISK_FILES];
extern Process process_table[MAX_PROCESSES];

extern "C" {

uint32_t get_file_count();

void draw_explorer() {
    if (!explorer_open) return;

    // Окно проводника
    draw_rect(explorer_x, explorer_y, 420, 320, 0xDDDDDD); 
    draw_rect(explorer_x, explorer_y, 420, 28, 0x444444);  
    draw_string(explorer_x + 10, explorer_y + 8, "FILE EXPLORER", 0xFFFFFF);
    
    // Крестик закрытия
    draw_rect(explorer_x + 395, explorer_y + 5, 20, 18, 0xAA0000);
    draw_string(explorer_x + 402, explorer_y + 8, "X", 0xFFFFFF);

    // ИСПРАВЛЕНО: Кнопочный надежный скроллбар со стрелочками
    draw_rect(explorer_x + 395, explorer_y + 28, 25, 25, 0x999999);  // Кнопка ВВЕРХ
    draw_string(explorer_x + 404, explorer_y + 34, "^", 0x000000);
    
    draw_rect(explorer_x + 395, explorer_y + 53, 25, 207, 0xBBBBBB); // Трек
    
    // Индикатор положения скролла
    draw_rect(explorer_x + 397, explorer_y + 55 + (explorer_scroll * 15), 21, 25, 0x777777);

    draw_rect(explorer_x + 395, explorer_y + 260, 25, 25, 0x999999); // Кнопка ВНИЗ
    draw_string(explorer_x + 404, explorer_y + 266, "V", 0x000000);

    // Кнопка создания файла
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

// НОВЫЙ ГРАФИЧЕСКИЙ ПРИЛАТ: Текстовый редактор (Блокнот)
void draw_notepad() {
    if (!notepad_open) return;

    // Шапка и рамка окна
    draw_rect(notepad_x, notepad_y, 400, 250, 0xF5F5F5); // Светло-серый фон блокнота
    draw_rect(notepad_x, notepad_y, 400, 28, 0x0055AA);   // Синяя шапка
    draw_string(notepad_x + 10, notepad_y + 8, "EDIT:", 0xFFFFFF);
    draw_string(notepad_x + 60, notepad_y + 8, current_editing_file, 0xFFFF00); // Жёлтое имя файла

    // Крестик закрытия
    draw_rect(notepad_x + 375, notepad_y + 5, 20, 18, 0xAA0000);
    draw_string(notepad_x + 382, notepad_y + 8, "X", 0xFFFFFF);

    // Рабочая текстовая зона (Белый лист бумаги)
    draw_rect(notepad_x + 10, notepad_y + 38, 380, 170, 0xFFFFFF);
    
    // Выводим текст, который сейчас напечатан в буфере
    if (notepad_buffer[0] == '\0') {
        draw_string(notepad_x + 15, notepad_y + 45, "TYPE TEXT HERE...", 0xCCCCCC);
    } else {
        draw_string(notepad_x + 15, notepad_y + 45, notepad_buffer, 0x000000);
    }

    // Кнопка сохранения изменений на диск
    draw_rect(notepad_x + 15, notepad_y + 215, 80, 24, 0x00AA55);
    draw_string(notepad_x + 35, notepad_y + 223, "SAVE", 0xFFFFFF);
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