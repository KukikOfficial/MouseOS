#include "include/fs.h"
#include "include/ata.h"
#include "include/vga.h"
#include "include/memory.h"

SuperBlock sb;
DiskFileEntry file_table[MAX_DISK_FILES];

// Вспомогательная функция для сравнения имен
bool fs_str_match(const char* s1, const char* s2) {
    int i = 0;
    while (s1[i] && s2[i] && s2[i] != ' ' && s2[i] != '\n') {
        if (s1[i] != s2[i]) return false;
        i++;
    }
    return (s1[i] == '\0');
}

void init_mouse_fs() {
    // Читаем Суперблок из Сектора 1
    ata_read_sector(1, (uint16_t*)&sb);

    if (sb.magic != MOUSE_FS_MAGIC) {
        terminal_writestring("Mouse Core: Disk not formatted. Initializing MouseFS...\n");
        sb.magic = MOUSE_FS_MAGIC;
        sb.file_count = 0;
        // Записываем чистый суперблок обратно
        ata_write_sector(1, (uint16_t*)&sb);
        
        // Очищаем таблицу файлов (Сектор 2)
        for(int i=0; i<MAX_DISK_FILES; i++) file_table[i].flags = 0;
        ata_write_sector(2, (uint16_t*)file_table);
    } else {
        terminal_writestring("Mouse Core: MouseFS detected. Loading file table...\n");
        ata_read_sector(2, (uint16_t*)file_table);
    }
}

void write_to_disk(const char* name, const char* content) {
    if (sb.file_count >= MAX_DISK_FILES) {
        terminal_writestring("Error: Disk Full!\n");
        return;
    }

    uint32_t idx = sb.file_count;
    
    // Заполняем запись
    int i = 0;
    while (name[i] && name[i] != ' ' && i < 19) {
        file_table[idx].name[i] = name[i];
        i++;
    }
    file_table[idx].name[i] = '\0';
    file_table[idx].start_lba = 10 + idx; // Данные начинаются с 10 сектора
    file_table[idx].flags = 1;

    // Пишем данные на диск
    uint16_t buffer[256];
    for(int j=0; j<256; j++) buffer[j] = 0;
    
    char* ptr = (char*)buffer;
    for(int j=0; content[j] != '\0' && j < 510; j++) ptr[j] = content[j];

    ata_write_sector(file_table[idx].start_lba, buffer);

    // Обновляем систему
    sb.file_count++;
    ata_write_sector(1, (uint16_t*)&sb);
    ata_write_sector(2, (uint16_t*)file_table);
    
    terminal_writestring("Mouse Core: File saved to physical disk.\n");
}

void list_files() {
    terminal_writestring("Files on MouseFS:\n");
    if (sb.file_count == 0) terminal_writestring(" (empty)\n");
    for (uint32_t i = 0; i < sb.file_count; i++) {
        if (file_table[i].flags == 1) {
            terminal_writestring(" * ");
            terminal_writestring(file_table[i].name);
            terminal_writestring("\n");
        }
    }
}

void read_from_disk(const char* name) {
    for (uint32_t i = 0; i < sb.file_count; i++) {
        if (file_table[i].flags == 1 && fs_str_match(file_table[i].name, name)) {
            uint16_t buffer[256];
            ata_read_sector(file_table[i].start_lba, buffer);
            terminal_writestring((char*)buffer);
            terminal_writestring("\n");
            return;
        }
    }
    terminal_writestring("Error: File not found on disk.\n");
}
